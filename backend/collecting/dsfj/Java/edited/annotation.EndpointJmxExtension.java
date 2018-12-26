

package org.springframework.boot.actuate.endpoint.jmx.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.EndpointExtension;
import org.springframework.core.annotation.AliasFor;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@EndpointExtension(filter = JmxEndpointFilter.class)
public @interface EndpointJmxExtension {

	
	@AliasFor(annotation = EndpointExtension.class)
	Class<?> endpoint();

}
