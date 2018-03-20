

package org.elasticsearch.painless;

import java.lang.invoke.MethodType;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.function.Function;

import static java.util.Collections.unmodifiableList;
import static org.elasticsearch.painless.WriterConstants.NEEDS_PARAMETER_METHOD_TYPE;


public class ScriptClassInfo {

    private final Class<?> baseClass;
    private final org.objectweb.asm.commons.Method executeMethod;
    private final Class<?> executeMethodReturnType;
    private final List<MethodArgument> executeArguments;
    private final List<org.objectweb.asm.commons.Method> needsMethods;
    private final List<org.objectweb.asm.commons.Method> getMethods;
    private final List<Class<?>> getReturns;

    public ScriptClassInfo(Definition definition, Class<?> baseClass) {
        this.baseClass = baseClass;

                java.lang.reflect.Method executeMethod = null;
        List<org.objectweb.asm.commons.Method> needsMethods = new ArrayList<>();
        List<org.objectweb.asm.commons.Method> getMethods = new ArrayList<>();
        List<Class<?>> getReturns = new ArrayList<>();
        for (java.lang.reflect.Method m : baseClass.getMethods()) {
            if (m.isDefault()) {
                continue;
            }
            if (m.getName().equals("execute")) {
                if (executeMethod == null) {
                    executeMethod = m;
                } else {
                    throw new IllegalArgumentException(
                            "Painless can only implement interfaces that have a single method named [execute] but [" + baseClass.getName()
                                    + "] has more than one.");
                }
            }
            if (m.getName().startsWith("needs") && m.getReturnType() == boolean.class && m.getParameterTypes().length == 0) {
                needsMethods.add(new org.objectweb.asm.commons.Method(m.getName(), NEEDS_PARAMETER_METHOD_TYPE.toMethodDescriptorString()));
            }
            if (m.getName().startsWith("get") && m.getName().equals("getClass") == false && Modifier.isStatic(m.getModifiers()) == false) {
                getReturns.add(
                    definitionTypeForClass(definition, m.getReturnType(), componentType -> "[" + m.getName() + "] has unknown return type ["
                    + componentType.getName() + "]. Painless can only support getters with return types that are whitelisted."));

                getMethods.add(new org.objectweb.asm.commons.Method(m.getName(),
                    MethodType.methodType(m.getReturnType()).toMethodDescriptorString()));

            }
        }
        MethodType methodType = MethodType.methodType(executeMethod.getReturnType(), executeMethod.getParameterTypes());
        this.executeMethod = new org.objectweb.asm.commons.Method(executeMethod.getName(), methodType.toMethodDescriptorString());
        executeMethodReturnType = definitionTypeForClass(definition, executeMethod.getReturnType(),
                componentType -> "Painless can only implement execute methods returning a whitelisted type but [" + baseClass.getName()
                        + "#execute] returns [" + componentType.getName() + "] which isn't whitelisted.");

                List<MethodArgument> arguments = new ArrayList<>();
        String[] argumentNamesConstant = readArgumentNamesConstant(baseClass);
        Class<?>[] types = executeMethod.getParameterTypes();
        if (argumentNamesConstant.length != types.length) {
            throw new IllegalArgumentException("[" + baseClass.getName() + "#ARGUMENTS] has length [2] but ["
                    + baseClass.getName() + "#execute] takes [1] argument.");
        }
        for (int arg = 0; arg < types.length; arg++) {
            arguments.add(methodArgument(definition, types[arg], argumentNamesConstant[arg]));
        }
        this.executeArguments = unmodifiableList(arguments);
        this.needsMethods = unmodifiableList(needsMethods);
        this.getMethods = unmodifiableList(getMethods);
        this.getReturns = unmodifiableList(getReturns);
    }

    
    public Class<?> getBaseClass() {
        return baseClass;
    }

    
    public org.objectweb.asm.commons.Method getExecuteMethod() {
        return executeMethod;
    }

    
    public Class<?> getExecuteMethodReturnType() {
        return executeMethodReturnType;
    }

    
    public List<MethodArgument> getExecuteArguments() {
        return executeArguments;
    }

    
    public List<org.objectweb.asm.commons.Method> getNeedsMethods() {
        return needsMethods;
    }

    
    public List<org.objectweb.asm.commons.Method> getGetMethods() {
        return getMethods;
    }

    
    public List<Class<?>> getGetReturns() {
        return getReturns;
    }

    
    public static class MethodArgument {
        private final Class<?> clazz;
        private final String name;

        public MethodArgument(Class<?> clazz, String name) {
            this.clazz = clazz;
            this.name = name;
        }

        public Class<?> getClazz() {
            return clazz;
        }

        public String getName() {
            return name;
        }
    }

    private MethodArgument methodArgument(Definition definition, Class<?> clazz, String argName) {
        Class<?> defClass = definitionTypeForClass(definition, clazz, componentType -> "[" + argName + "] is of unknown type ["
                + componentType.getName() + ". Painless interfaces can only accept arguments that are of whitelisted types.");
        return new MethodArgument(defClass, argName);
    }

    private static Class<?> definitionTypeForClass(Definition definition, Class<?> type,
            Function<Class<?>, String> unknownErrorMessageSource) {
        int dimensions = 0;
        Class<?> componentType = type;
        while (componentType.isArray()) {
            dimensions++;
            componentType = componentType.getComponentType();
        }
        Definition.Struct struct;
        if (componentType == Object.class) {
            struct = definition.getType("def").struct;
        } else {
            if (definition.RuntimeClassToStruct(componentType) == null) {
                throw new IllegalArgumentException(unknownErrorMessageSource.apply(componentType));
            }
            struct = definition.RuntimeClassToStruct(componentType);
        }
        return Definition.TypeToClass(definition.getType(struct, dimensions));
    }

    private static String[] readArgumentNamesConstant(Class<?> iface) {
        Field argumentNamesField;
        try {
            argumentNamesField = iface.getField("PARAMETERS");
        } catch (NoSuchFieldException e) {
            throw new IllegalArgumentException("Painless needs a constant [String[] PARAMETERS] on all interfaces it implements with the "
                    + "names of the method arguments but [" + iface.getName() + "] doesn't have one.", e);
        }
        if (false == argumentNamesField.getType().equals(String[].class)) {
            throw new IllegalArgumentException("Painless needs a constant [String[] PARAMETERS] on all interfaces it implements with the "
                    + "names of the method arguments but [" + iface.getName() + "] doesn't have one.");
        }
        try {
            return (String[]) argumentNamesField.get(null);
        } catch (IllegalArgumentException | IllegalAccessException e) {
            throw new IllegalArgumentException("Error trying to read [" + iface.getName() + "#ARGUMENTS]", e);
        }
    }
}
