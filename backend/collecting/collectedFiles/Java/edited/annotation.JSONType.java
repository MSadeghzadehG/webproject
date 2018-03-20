package com.alibaba.fastjson.annotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import com.alibaba.fastjson.PropertyNamingStrategy;
import com.alibaba.fastjson.parser.Feature;
import com.alibaba.fastjson.serializer.SerializerFeature;


@Retention(RetentionPolicy.RUNTIME)
@Target({ ElementType.TYPE })
public @interface JSONType {

    boolean asm() default true;

    String[] orders() default {};

    
    String[] includes() default {};

    String[] ignores() default {};

    SerializerFeature[] serialzeFeatures() default {};
    Feature[] parseFeatures() default {};
    
    boolean alphabetic() default true;
    
    Class<?> mappingTo() default Void.class;
    
    Class<?> builder() default Void.class;
    
    
    String typeName() default "";

    
    String typeKey() default "";
    
    
    Class<?>[] seeAlso() default{};
    
    
    Class<?> serializer() default Void.class;
    
    
    Class<?> deserializer() default Void.class;

    boolean serializeEnumAsJavaBean() default false;

    PropertyNamingStrategy naming() default PropertyNamingStrategy.CamelCase;
}
