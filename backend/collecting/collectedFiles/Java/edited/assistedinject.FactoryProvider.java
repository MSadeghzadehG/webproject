

package org.elasticsearch.common.inject.assistedinject;

import org.elasticsearch.common.inject.ConfigurationException;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.Injector;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.Provider;
import org.elasticsearch.common.inject.TypeLiteral;
import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.spi.Dependency;
import org.elasticsearch.common.inject.spi.HasDependencies;
import org.elasticsearch.common.inject.spi.Message;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import static java.util.Collections.emptyMap;
import static java.util.Collections.singleton;
import static java.util.Collections.unmodifiableSet;


public class FactoryProvider<F> implements Provider<F>, HasDependencies {

    

    private Injector injector;

    private final TypeLiteral<F> factoryType;
    private final Map<Method, AssistedConstructor<?>> factoryMethodToConstructor;

    private FactoryProvider(TypeLiteral<F> factoryType,
                            Map<Method, AssistedConstructor<?>> factoryMethodToConstructor) {
        this.factoryType = factoryType;
        this.factoryMethodToConstructor = factoryMethodToConstructor;
        checkDeclaredExceptionsMatch();
    }

    private void checkDeclaredExceptionsMatch() {
        for (Map.Entry<Method, AssistedConstructor<?>> entry : factoryMethodToConstructor.entrySet()) {
            for (Class<?> constructorException : entry.getValue().getDeclaredExceptions()) {
                if (!isConstructorExceptionCompatibleWithFactoryExeception(
                        constructorException, entry.getKey().getExceptionTypes())) {
                    throw newConfigurationException("Constructor %s declares an exception, but no compatible "
                            + "exception is thrown by the factory method %s", entry.getValue(), entry.getKey());
                }
            }
        }
    }

    private boolean isConstructorExceptionCompatibleWithFactoryExeception(
            Class<?> constructorException, Class<?>[] factoryExceptions) {
        for (Class<?> factoryException : factoryExceptions) {
            if (factoryException.isAssignableFrom(constructorException)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public Set<Dependency<?>> getDependencies() {
        Set<Dependency<?>> dependencies = new HashSet<>();
        for (AssistedConstructor<?> constructor : factoryMethodToConstructor.values()) {
            for (Parameter parameter : constructor.getAllParameters()) {
                if (!parameter.isProvidedByFactory()) {
                    dependencies.add(Dependency.get(parameter.getPrimaryBindingKey()));
                }
            }
        }
        return unmodifiableSet(dependencies);
    }

    @Override
    public F get() {
        InvocationHandler invocationHandler = new InvocationHandler() {
            @Override
            public Object invoke(Object proxy, Method method, Object[] creationArgs) throws Throwable {
                                if (method.getDeclaringClass().equals(Object.class)) {
                    return method.invoke(this, creationArgs);
                }

                AssistedConstructor<?> constructor = factoryMethodToConstructor.get(method);
                Object[] constructorArgs = gatherArgsForConstructor(constructor, creationArgs);
                Object objectToReturn = constructor.newInstance(constructorArgs);
                injector.injectMembers(objectToReturn);
                return objectToReturn;
            }

            public Object[] gatherArgsForConstructor(
                    AssistedConstructor<?> constructor,
                    Object[] factoryArgs) {
                int numParams = constructor.getAllParameters().size();
                int argPosition = 0;
                Object[] result = new Object[numParams];

                for (int i = 0; i < numParams; i++) {
                    Parameter parameter = constructor.getAllParameters().get(i);
                    if (parameter.isProvidedByFactory()) {
                        result[i] = factoryArgs[argPosition];
                        argPosition++;
                    } else {
                        result[i] = parameter.getValue(injector);
                    }
                }
                return result;
            }
        };

        @SuppressWarnings("unchecked")                 Class<F> factoryRawType = (Class) factoryType.getRawType();
        return factoryRawType.cast(Proxy.newProxyInstance(factoryRawType.getClassLoader(),
                new Class[]{factoryRawType}, invocationHandler));
    }

    private static ConfigurationException newConfigurationException(String format, Object... args) {
        return new ConfigurationException(singleton(new Message(Errors.format(format, args))));
    }
}
