

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.MoreTypes;
import org.elasticsearch.common.inject.util.Types;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

import static org.elasticsearch.common.inject.internal.MoreTypes.canonicalize;


public class TypeLiteral<T> {

    final Class<? super T> rawType;
    final Type type;
    final int hashCode;

    
    @SuppressWarnings("unchecked")
    protected TypeLiteral() {
        this.type = getSuperclassTypeParameter(getClass());
        this.rawType = (Class<? super T>) MoreTypes.getRawType(type);
        this.hashCode = MoreTypes.hashCode(type);
    }

    
    @SuppressWarnings("unchecked")
    TypeLiteral(Type type) {
        this.type = canonicalize(Objects.requireNonNull(type, "type"));
        this.rawType = (Class<? super T>) MoreTypes.getRawType(this.type);
        this.hashCode = MoreTypes.hashCode(this.type);
    }

    
    static Type getSuperclassTypeParameter(Class<?> subclass) {
        Type superclass = subclass.getGenericSuperclass();
        if (superclass instanceof Class) {
            throw new RuntimeException("Missing type parameter.");
        }
        ParameterizedType parameterized = (ParameterizedType) superclass;
        return canonicalize(parameterized.getActualTypeArguments()[0]);
    }

    
    static TypeLiteral<?> fromSuperclassTypeParameter(Class<?> subclass) {
        return new TypeLiteral<Object>(getSuperclassTypeParameter(subclass));
    }

    
    public final Class<? super T> getRawType() {
        return rawType;
    }

    
    public final Type getType() {
        return type;
    }

    
    @SuppressWarnings("unchecked")
    final TypeLiteral<Provider<T>> providerType() {
                        return (TypeLiteral<Provider<T>>) get(Types.providerOf(getType()));
    }

    @Override
    public final int hashCode() {
        return this.hashCode;
    }

    @Override
    public final boolean equals(Object o) {
        return o instanceof TypeLiteral<?>
                && MoreTypes.equals(type, ((TypeLiteral) o).type);
    }

    @Override
    public final String toString() {
        return MoreTypes.toString(type);
    }

    
    public static TypeLiteral<?> get(Type type) {
        return new TypeLiteral<Object>(type);
    }

    
    public static <T> TypeLiteral<T> get(Class<T> type) {
        return new TypeLiteral<>(type);
    }


    
    private List<TypeLiteral<?>> resolveAll(Type[] types) {
        TypeLiteral<?>[] result = new TypeLiteral<?>[types.length];
        for (int t = 0; t < types.length; t++) {
            result[t] = resolve(types[t]);
        }
        return Arrays.asList(result);
    }

    
    TypeLiteral<?> resolve(Type toResolve) {
        return TypeLiteral.get(resolveType(toResolve));
    }

    Type resolveType(Type toResolve) {
                while (true) {
            if (toResolve instanceof TypeVariable) {
                TypeVariable original = (TypeVariable) toResolve;
                toResolve = MoreTypes.resolveTypeVariable(type, rawType, original);
                if (toResolve == original) {
                    return toResolve;
                }

            } else if (toResolve instanceof GenericArrayType) {
                GenericArrayType original = (GenericArrayType) toResolve;
                Type componentType = original.getGenericComponentType();
                Type newComponentType = resolveType(componentType);
                return componentType == newComponentType
                        ? original
                        : Types.arrayOf(newComponentType);

            } else if (toResolve instanceof ParameterizedType) {
                ParameterizedType original = (ParameterizedType) toResolve;
                Type ownerType = original.getOwnerType();
                Type newOwnerType = resolveType(ownerType);
                boolean changed = newOwnerType != ownerType;

                Type[] args = original.getActualTypeArguments();
                for (int t = 0, length = args.length; t < length; t++) {
                    Type resolvedTypeArgument = resolveType(args[t]);
                    if (resolvedTypeArgument != args[t]) {
                        if (!changed) {
                            args = args.clone();
                            changed = true;
                        }
                        args[t] = resolvedTypeArgument;
                    }
                }

                return changed
                        ? Types.newParameterizedTypeWithOwner(newOwnerType, original.getRawType(), args)
                        : original;

            } else if (toResolve instanceof WildcardType) {
                WildcardType original = (WildcardType) toResolve;
                Type[] originalLowerBound = original.getLowerBounds();
                Type[] originalUpperBound = original.getUpperBounds();

                if (originalLowerBound.length == 1) {
                    Type lowerBound = resolveType(originalLowerBound[0]);
                    if (lowerBound != originalLowerBound[0]) {
                        return Types.supertypeOf(lowerBound);
                    }
                } else if (originalUpperBound.length == 1) {
                    Type upperBound = resolveType(originalUpperBound[0]);
                    if (upperBound != originalUpperBound[0]) {
                        return Types.subtypeOf(upperBound);
                    }
                }
                return original;

            } else {
                return toResolve;
            }
        }
    }

    
    public TypeLiteral<?> getSupertype(Class<?> supertype) {
        if (!supertype.isAssignableFrom(rawType)) {
            throw new IllegalArgumentException(supertype + " is not a supertype of " + type);
        }
        return resolve(MoreTypes.getGenericSupertype(type, rawType, supertype));
    }

    
    public TypeLiteral<?> getFieldType(Field field) {
        if (!field.getDeclaringClass().isAssignableFrom(rawType)) {
            throw new IllegalArgumentException(field + " is not defined by a supertype of " + type);
        }
        return resolve(field.getGenericType());
    }

    
    public List<TypeLiteral<?>> getParameterTypes(Member methodOrConstructor) {
        Type[] genericParameterTypes;

        if (methodOrConstructor instanceof Method) {
            Method method = (Method) methodOrConstructor;
            if (!method.getDeclaringClass().isAssignableFrom(rawType)) {
                throw new IllegalArgumentException(method + " is not defined by a supertype of " + type);
            }
            genericParameterTypes = method.getGenericParameterTypes();

        } else if (methodOrConstructor instanceof Constructor) {
            Constructor constructor = (Constructor) methodOrConstructor;
            if (!constructor.getDeclaringClass().isAssignableFrom(rawType)) {
                throw new IllegalArgumentException(constructor + " does not construct a supertype of " + type);
            }

            genericParameterTypes = constructor.getGenericParameterTypes();

        } else {
            throw new IllegalArgumentException("Not a method or a constructor: " + methodOrConstructor);
        }

        return resolveAll(genericParameterTypes);
    }

    
    public List<TypeLiteral<?>> getExceptionTypes(Member methodOrConstructor) {
        Type[] genericExceptionTypes;

        if (methodOrConstructor instanceof Method) {
            Method method = (Method) methodOrConstructor;
            if (!method.getDeclaringClass().isAssignableFrom(rawType)) {
                throw new IllegalArgumentException(method + " is not defined by a supertype of " + type);
            }

            genericExceptionTypes = method.getGenericExceptionTypes();

        } else if (methodOrConstructor instanceof Constructor) {
            Constructor<?> constructor = (Constructor<?>) methodOrConstructor;
            if (!constructor.getDeclaringClass().isAssignableFrom(rawType)) {
                throw new IllegalArgumentException(constructor + " does not construct a supertype of " + type);
            }
            genericExceptionTypes = constructor.getGenericExceptionTypes();

        } else {
            throw new IllegalArgumentException("Not a method or a constructor: " + methodOrConstructor);
        }

        return resolveAll(genericExceptionTypes);
    }

    
    public TypeLiteral<?> getReturnType(Method method) {
        if (!method.getDeclaringClass().isAssignableFrom(rawType)) {
            throw new IllegalArgumentException(method + " is not defined by a supertype of " + type);
        }

        return resolve(method.getGenericReturnType());
    }
}
