
package com.alibaba.dubbo.config.spring.context.annotation;

import org.springframework.context.annotation.Import;

import java.lang.annotation.*;


@Target({ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Import(DubboConfigBindingsRegistrar.class)
public @interface EnableDubboConfigBindings {

    
    EnableDubboConfigBinding[] value();

}
