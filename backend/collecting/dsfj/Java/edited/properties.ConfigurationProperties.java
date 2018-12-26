

package org.springframework.boot.context.properties;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.core.annotation.AliasFor;


@Target({ ElementType.TYPE, ElementType.METHOD })
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface ConfigurationProperties {

	
	@AliasFor("prefix")
	String value() default "";

	
	@AliasFor("value")
	String prefix() default "";

	
	boolean ignoreInvalidFields() default false;

	
	boolean ignoreUnknownFields() default true;

}
