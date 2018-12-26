
package com.alibaba.dubbo.common.extension;

import com.alibaba.dubbo.common.URL;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;


@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE, ElementType.METHOD})
public @interface Activate {
    
    String[] group() default {};

    
    String[] value() default {};

    
    String[] before() default {};

    
    String[] after() default {};

    
    int order() default 0;
}