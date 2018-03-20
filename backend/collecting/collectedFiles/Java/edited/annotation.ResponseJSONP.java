package com.alibaba.fastjson.support.spring.annotation;

import org.springframework.web.bind.annotation.ResponseBody;

import java.lang.annotation.*;


@Documented
@Target({ElementType.TYPE,ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@ResponseBody
public @interface ResponseJSONP {
    
    String callback() default "callback";
}
