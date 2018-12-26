

package org.elasticsearch.common.inject.spi;

import org.elasticsearch.common.inject.ConfigurationException;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.TypeLiteral;
import org.elasticsearch.common.inject.internal.Annotations;
import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.internal.ErrorsException;
import org.elasticsearch.common.inject.internal.MoreTypes;
import org.elasticsearch.common.inject.internal.Nullability;

import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import static java.util.Collections.unmodifiableSet;
import static org.elasticsearch.common.inject.internal.MoreTypes.getRawType;


public final class InjectionPoint {

    private final boolean optional;
    private final Member member;
    private final List<Dependency<?>> dependencies;

    private InjectionPoint(Member member,
                           List<Dependency<?>> dependencies, boolean optional) {
        this.member = member;
        this.dependencies = dependencies;
        this.optional = optional;
    }

    InjectionPoint(TypeLiteral<?> type, Method method) {
        this.member = method;

        Inject inject = method.getAnnotation(Inject.class);
        this.optional = inject.optional();

        this.dependencies = forMember(method, type, method.getParameterAnnotations());
    }

    InjectionPoint(TypeLiteral<?> type, Constructor<?> constructor) {
        this.member = constructor;
        this.optional = false;
        this.dependencies = forMember(constructor, type, constructor.getParameterAnnotations());
    }

    InjectionPoint(TypeLiteral<?> type, Field field) {
        this.member = field;

        Inject inject = field.getAnnotation(Inject.class);
        this.optional = inject.optional();

        Annotation[] annotations = field.getAnnotations();

        Errors errors = new Errors(field);
        Key<?> key = null;
        try {
            key = Annotations.getKey(type.getFieldType(field), field, annotations, errors);
        } catch (ErrorsException e) {
            errors.merge(e.getErrors());
        }
        errors.throwConfigurationExceptionIfErrorsExist();

        this.dependencies = Collections.<Dependency<?>>singletonList(
            newDependency(key, Nullability.allowsNull(annotations), -1));
    }

    private List<Dependency<?>> forMember(Member member, TypeLiteral<?> type,
                                                   Annotation[][] parameterAnnotations) {
        Errors errors = new Errors(member);
        Iterator<Annotation[]> annotationsIterator = Arrays.asList(parameterAnnotations).iterator();

        List<Dependency<?>> dependencies = new ArrayList<>();
        int index = 0;

        for (TypeLiteral<?> parameterType : type.getParameterTypes(member)) {
            try {
                Annotation[] paramAnnotations = annotationsIterator.next();
                Key<?> key = Annotations.getKey(parameterType, member, paramAnnotations, errors);
                dependencies.add(newDependency(key, Nullability.allowsNull(paramAnnotations), index));
                index++;
            } catch (ErrorsException e) {
                errors.merge(e.getErrors());
            }
        }

        errors.throwConfigurationExceptionIfErrorsExist();
        return Collections.unmodifiableList(dependencies);
    }

        private <T> Dependency<T> newDependency(Key<T> key, boolean allowsNull, int parameterIndex) {
        return new Dependency<>(this, key, allowsNull, parameterIndex);
    }

    
    public Member getMember() {
        return member;
    }

    
    public List<Dependency<?>> getDependencies() {
        return dependencies;
    }

    
    public boolean isOptional() {
        return optional;
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof InjectionPoint
                && member.equals(((InjectionPoint) o).member);
    }

    @Override
    public int hashCode() {
        return member.hashCode();
    }

    @Override
    public String toString() {
        return MoreTypes.toString(member);
    }

    
    public static InjectionPoint forConstructorOf(TypeLiteral<?> type) {
        Class<?> rawType = getRawType(type.getType());
        Errors errors = new Errors(rawType);

        Constructor<?> injectableConstructor = null;
        for (Constructor<?> constructor : rawType.getConstructors()) {
            Inject inject = constructor.getAnnotation(Inject.class);
            if (inject != null) {
                if (inject.optional()) {
                    errors.optionalConstructor(constructor);
                }

                if (injectableConstructor != null) {
                    errors.tooManyConstructors(rawType);
                }

                injectableConstructor = constructor;
                checkForMisplacedBindingAnnotations(injectableConstructor, errors);
            }
        }

        errors.throwConfigurationExceptionIfErrorsExist();

        if (injectableConstructor != null) {
            return new InjectionPoint(type, injectableConstructor);
        }

                try {
            Constructor<?> noArgConstructor = rawType.getConstructor();

                        if (Modifier.isPrivate(noArgConstructor.getModifiers())
                    && !Modifier.isPrivate(rawType.getModifiers())) {
                errors.missingConstructor(rawType);
                throw new ConfigurationException(errors.getMessages());
            }

            checkForMisplacedBindingAnnotations(noArgConstructor, errors);
            return new InjectionPoint(type, noArgConstructor);
        } catch (NoSuchMethodException e) {
            errors.missingConstructor(rawType);
            throw new ConfigurationException(errors.getMessages());
        }
    }

    
    public static InjectionPoint forConstructorOf(Class<?> type) {
        return forConstructorOf(TypeLiteral.get(type));
    }

    
    public static Set<InjectionPoint> forStaticMethodsAndFields(TypeLiteral type) {
        Set<InjectionPoint> result = new HashSet<>();
        Errors errors = new Errors();

        addInjectionPoints(type, Factory.FIELDS, true, result, errors);
        addInjectionPoints(type, Factory.METHODS, true, result, errors);

        result = unmodifiableSet(result);
        if (errors.hasErrors()) {
            throw new ConfigurationException(errors.getMessages()).withPartialValue(result);
        }
        return result;
    }

    
    public static Set<InjectionPoint> forStaticMethodsAndFields(Class<?> type) {
        return forStaticMethodsAndFields(TypeLiteral.get(type));
    }

    
    public static Set<InjectionPoint> forInstanceMethodsAndFields(TypeLiteral<?> type) {
        Set<InjectionPoint> result = new HashSet<>();
        Errors errors = new Errors();

                addInjectionPoints(type, Factory.FIELDS, false, result, errors);
        addInjectionPoints(type, Factory.METHODS, false, result, errors);

        result = unmodifiableSet(result);
        if (errors.hasErrors()) {
            throw new ConfigurationException(errors.getMessages()).withPartialValue(result);
        }
        return result;
    }

    
    public static Set<InjectionPoint> forInstanceMethodsAndFields(Class<?> type) {
        return forInstanceMethodsAndFields(TypeLiteral.get(type));
    }

    private static void checkForMisplacedBindingAnnotations(Member member, Errors errors) {
        Annotation misplacedBindingAnnotation = Annotations.findBindingAnnotation(
                errors, member, ((AnnotatedElement) member).getAnnotations());
        if (misplacedBindingAnnotation == null) {
            return;
        }

                        if (member instanceof Method) {
            try {
                if (member.getDeclaringClass().getField(member.getName()) != null) {
                    return;
                }
            } catch (NoSuchFieldException ignore) {
            }
        }

        errors.misplacedBindingAnnotation(member, misplacedBindingAnnotation);
    }

    private static <M extends Member & AnnotatedElement> void addInjectionPoints(TypeLiteral<?> type,
                                                                                 Factory<M> factory, boolean statics, Collection<InjectionPoint> injectionPoints,
                                                                                 Errors errors) {
        if (type.getType() == Object.class) {
            return;
        }

                TypeLiteral<?> superType = type.getSupertype(type.getRawType().getSuperclass());
        addInjectionPoints(superType, factory, statics, injectionPoints, errors);

                addInjectorsForMembers(type, factory, statics, injectionPoints, errors);
    }

    private static <M extends Member & AnnotatedElement> void addInjectorsForMembers(
            TypeLiteral<?> typeLiteral, Factory<M> factory, boolean statics,
            Collection<InjectionPoint> injectionPoints, Errors errors) {
        for (M member : factory.getMembers(getRawType(typeLiteral.getType()))) {
            if (isStatic(member) != statics) {
                continue;
            }

            Inject inject = member.getAnnotation(Inject.class);
            if (inject == null) {
                continue;
            }

            try {
                injectionPoints.add(factory.create(typeLiteral, member, errors));
            } catch (ConfigurationException ignorable) {
                if (!inject.optional()) {
                    errors.merge(ignorable.getErrorMessages());
                }
            }
        }
    }

    private static boolean isStatic(Member member) {
        return Modifier.isStatic(member.getModifiers());
    }

    private interface Factory<M extends Member & AnnotatedElement> {
        Factory<Field> FIELDS = new Factory<Field>() {
            @Override
            public Field[] getMembers(Class<?> type) {
                return type.getFields();
            }

            @Override
            public InjectionPoint create(TypeLiteral<?> typeLiteral, Field member, Errors errors) {
                return new InjectionPoint(typeLiteral, member);
            }
        };

        Factory<Method> METHODS = new Factory<Method>() {
            @Override
            public Method[] getMembers(Class<?> type) {
                return type.getMethods();
            }

            @Override
            public InjectionPoint create(TypeLiteral<?> typeLiteral, Method member, Errors errors) {
                checkForMisplacedBindingAnnotations(member, errors);
                return new InjectionPoint(typeLiteral, member);
            }
        };

        M[] getMembers(Class<?> type);

        InjectionPoint create(TypeLiteral<?> typeLiteral, M member, Errors errors);
    }
}
