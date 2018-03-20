

package org.springframework.boot.actuate.endpoint.jmx.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.FilteredEndpoint;
import org.springframework.core.annotation.AliasFor;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Endpoint
@FilteredEndpoint(JmxEndpointFilter.class)
public @interface JmxEndpoint {

	
	@AliasFor(annotation = Endpoint.class)
	String id() default "";

	
	@AliasFor(annotation = Endpoint.class)
	boolean enableByDefault() default true;

}
