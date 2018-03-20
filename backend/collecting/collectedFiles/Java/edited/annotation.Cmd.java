
package com.alibaba.dubbo.qos.command.annotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;


@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Cmd {

    
    String name();

    
    String summary();

    
    String[] example() default {};

    
    int sort() default 0;
}
