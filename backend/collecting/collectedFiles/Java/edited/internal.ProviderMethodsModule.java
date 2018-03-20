

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.Module;
import org.elasticsearch.common.inject.Provider;
import org.elasticsearch.common.inject.Provides;
import org.elasticsearch.common.inject.TypeLiteral;
import org.elasticsearch.common.inject.spi.Dependency;
import org.elasticsearch.common.inject.spi.Message;
import org.elasticsearch.common.inject.util.Modules;

import java.lang.annotation.Annotation;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import static java.util.Collections.unmodifiableSet;


public final class ProviderMethodsModule implements Module {
    private final Object delegate;
    private final TypeLiteral<?> typeLiteral;

    private ProviderMethodsModule(Object delegate) {
        this.delegate = Objects.requireNonNull(delegate, "delegate");
        this.typeLiteral = TypeLiteral.get(this.delegate.getClass());
    }

    
    public static Module forModule(Module module) {
        return forObject(module);
    }

    
    public static Module forObject(Object object) {
                if (object instanceof ProviderMethodsModule) {
            return Modules.EMPTY_MODULE;
        }

        return new ProviderMethodsModule(object);
    }

    @Override
    public synchronized void configure(Binder binder) {
        for (ProviderMethod<?> providerMethod : getProviderMethods(binder)) {
            providerMethod.configure(binder);
        }
    }

    public List<ProviderMethod<?>> getProviderMethods(Binder binder) {
        List<ProviderMethod<?>> result = new ArrayList<>();
        for (Class<?> c = delegate.getClass(); c != Object.class; c = c.getSuperclass()) {
            for (Method method : c.getMethods()) {
                if (method.getAnnotation(Provides.class) != null) {
                    result.add(createProviderMethod(binder, method));
                }
            }
        }
        return result;
    }

    <T> ProviderMethod<T> createProviderMethod(Binder binder, final Method method) {
        binder = binder.withSource(method);
        Errors errors = new Errors(method);

                Set<Dependency<?>> dependencies = new HashSet<>();
        List<Provider<?>> parameterProviders = new ArrayList<>();
        List<TypeLiteral<?>> parameterTypes = typeLiteral.getParameterTypes(method);
        Annotation[][] parameterAnnotations = method.getParameterAnnotations();
        for (int i = 0; i < parameterTypes.size(); i++) {
            Key<?> key = getKey(errors, parameterTypes.get(i), method, parameterAnnotations[i]);
            dependencies.add(Dependency.get(key));
            parameterProviders.add(binder.getProvider(key));
        }

        @SuppressWarnings("unchecked")                 TypeLiteral<T> returnType = (TypeLiteral<T>) typeLiteral.getReturnType(method);

        Key<T> key = getKey(errors, returnType, method, method.getAnnotations());
        Class<? extends Annotation> scopeAnnotation
                = Annotations.findScopeAnnotation(errors, method.getAnnotations());

        for (Message message : errors.getMessages()) {
            binder.addError(message);
        }

        return new ProviderMethod<>(key, method, delegate, unmodifiableSet(dependencies),
                parameterProviders, scopeAnnotation);
    }

    <T> Key<T> getKey(Errors errors, TypeLiteral<T> type, Member member, Annotation[] annotations) {
        Annotation bindingAnnotation = Annotations.findBindingAnnotation(errors, member, annotations);
        return bindingAnnotation == null ? Key.get(type) : Key.get(type, bindingAnnotation);
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof ProviderMethodsModule
                && ((ProviderMethodsModule) o).delegate == delegate;
    }

    @Override
    public int hashCode() {
        return delegate.hashCode();
    }
}
