

package org.elasticsearch.common;

import java.lang.reflect.Modifier;

public class Classes {

    
    private static final char PACKAGE_SEPARATOR = '.';

    
    public static String getPackageName(Class<?> clazz) {
        String className = clazz.getName();
        int lastDotIndex = className.lastIndexOf(PACKAGE_SEPARATOR);
        return (lastDotIndex != -1 ? className.substring(0, lastDotIndex) : "");
    }

    public static boolean isInnerClass(Class<?> clazz) {
        return !Modifier.isStatic(clazz.getModifiers())
                && clazz.getEnclosingClass() != null;
    }

    public static boolean isConcrete(Class<?> clazz) {
        int modifiers = clazz.getModifiers();
        return !clazz.isInterface() && !Modifier.isAbstract(modifiers);
    }

    private Classes() {}
}
