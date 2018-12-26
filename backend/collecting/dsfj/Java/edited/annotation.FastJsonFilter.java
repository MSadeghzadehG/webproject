package com.alibaba.fastjson.support.spring.annotation;

import java.lang.annotation.*;


@Target(ElementType.ANNOTATION_TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface FastJsonFilter {
    Class<?> clazz();
    String[] props();
}
