

package org.springframework.boot.actuate.endpoint.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.actuate.endpoint.EndpointFilter;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface FilteredEndpoint {

	
	Class<? extends EndpointFilter<?>> value();

}
