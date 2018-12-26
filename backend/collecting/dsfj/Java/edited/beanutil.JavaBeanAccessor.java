
package com.alibaba.dubbo.common.beanutil;

public enum JavaBeanAccessor {

    
    FIELD,
    
    METHOD,
    
    ALL;

    public static boolean isAccessByMethod(JavaBeanAccessor accessor) {
        return METHOD.equals(accessor) || ALL.equals(accessor);
    }

    public static boolean isAccessByField(JavaBeanAccessor accessor) {
        return FIELD.equals(accessor) || ALL.equals(accessor);
    }

}
