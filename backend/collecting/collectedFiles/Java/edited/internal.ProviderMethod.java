

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Exposed;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.PrivateBinder;
import org.elasticsearch.common.inject.Provider;
import org.elasticsearch.common.inject.spi.Dependency;
import org.elasticsearch.common.inject.spi.ProviderWithDependencies;

import java.lang.annotation.Annotation;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Set;


public class ProviderMethod<T> implements ProviderWithDependencies<T> {
    private final Key<T> key;
    private final Class<? extends Annotation> scopeAnnotation;
    private final Object instance;
    private final Method method;
    private final Set<Dependency<?>> dependencies;
    private final List<Provider<?>> parameterProviders;
    private final boolean exposed;

    
    ProviderMethod(Key<T> key, Method method, Object instance,
                   Set<Dependency<?>> dependencies, List<Provider<?>> parameterProviders,
                   Class<? extends Annotation> scopeAnnotation) {
        this.key = key;
        this.scopeAnnotation = scopeAnnotation;
        this.instance = instance;
        this.dependencies = dependencies;
        this.method = method;
        this.parameterProviders = parameterProviders;
        this.exposed = method.getAnnotation(Exposed.class) != null;
    }

    public Key<T> getKey() {
        return key;
    }

    public Method getMethod() {
        return method;
    }

        public Object getInstance() {
        return instance;
    }

    public void configure(Binder binder) {
        binder = binder.withSource(method);

        if (scopeAnnotation != null) {
            binder.bind(key).toProvider(this).in(scopeAnnotation);
        } else {
            binder.bind(key).toProvider(this);
        }

        if (exposed) {
                                    ((PrivateBinder) binder).expose(key);
        }
    }

    @Override
    public T get() {
        Object[] parameters = new Object[parameterProviders.size()];
        for (int i = 0; i < parameters.length; i++) {
            parameters[i] = parameterProviders.get(i).get();
        }

        try {
                        @SuppressWarnings({"unchecked", "UnnecessaryLocalVariable"})
            T result = (T) method.invoke(instance, parameters);
            return result;
        } catch (IllegalAccessException e) {
            throw new AssertionError(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public Set<Dependency<?>> getDependencies() {
        return dependencies;
    }
}
