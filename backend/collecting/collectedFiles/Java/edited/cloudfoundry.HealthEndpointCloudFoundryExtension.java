

package org.springframework.boot.actuate.autoconfigure.cloudfoundry;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.actuate.endpoint.annotation.EndpointExtension;
import org.springframework.boot.actuate.health.HealthEndpoint;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@EndpointExtension(filter = CloudFoundryEndpointFilter.class, endpoint = HealthEndpoint.class)
public @interface HealthEndpointCloudFoundryExtension {

}
