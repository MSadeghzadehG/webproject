

package org.elasticsearch.common.inject.assistedinject;

import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.TypeLiteral;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


class AssistedConstructor<T> {

    private final Constructor<T> constructor;
    private final ParameterListKey assistedParameters;
    private final List<Parameter> allParameters;

    @SuppressWarnings("unchecked")
    AssistedConstructor(Constructor<T> constructor, List<TypeLiteral<?>> parameterTypes) {
        this.constructor = constructor;

        Annotation[][] annotations = constructor.getParameterAnnotations();

        List<Type> typeList = new ArrayList<>();
        allParameters = new ArrayList<>(parameterTypes.size());

                for (int i = 0; i < parameterTypes.size(); i++) {
            Parameter parameter = new Parameter(parameterTypes.get(i).getType(), annotations[i]);
            allParameters.add(parameter);
            if (parameter.isProvidedByFactory()) {
                typeList.add(parameter.getType());
            }
        }
        this.assistedParameters = new ParameterListKey(typeList);
    }

    
    public ParameterListKey getAssistedParameters() {
        return assistedParameters;
    }

    
    public List<Parameter> getAllParameters() {
        return allParameters;
    }

    public Set<Class<?>> getDeclaredExceptions() {
        return new HashSet<>(Arrays.asList(constructor.getExceptionTypes()));
    }

    
    public T newInstance(Object[] args) throws Throwable {
        try {
            return constructor.newInstance(args);
        } catch (InvocationTargetException e) {
            throw e.getCause();
        }
    }

    @Override
    public String toString() {
        return constructor.toString();
    }
}
