package com.alibaba.fastjson.support.spring.annotation;

import java.lang.annotation.*;


@Target({ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface FastJsonView {
    FastJsonFilter[] include() default {};
    FastJsonFilter[] exclude() default {};
}
