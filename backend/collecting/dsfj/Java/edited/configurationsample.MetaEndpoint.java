

package org.springframework.boot.configurationsample;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.core.annotation.AliasFor;

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Endpoint
public @interface MetaEndpoint {

	@AliasFor(annotation = Endpoint.class)
	String id();

	@AliasFor(annotation = Endpoint.class)
	boolean enableByDefault() default true;

}
