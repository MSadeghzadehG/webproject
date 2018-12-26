
package com.alibaba.dubbo.common.serialize.support;

import java.util.LinkedHashSet;
import java.util.Set;

public abstract class SerializableClassRegistry {

    private static final Set<Class> registrations = new LinkedHashSet<Class>();

    
    public static void registerClass(Class clazz) {
        registrations.add(clazz);
    }

    public static Set<Class> getRegisteredClasses() {
        return registrations;
    }
}
