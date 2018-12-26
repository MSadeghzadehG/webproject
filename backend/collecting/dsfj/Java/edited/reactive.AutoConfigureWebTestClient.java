

package org.springframework.boot.test.autoconfigure.web.reactive;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.time.Duration;

import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.test.autoconfigure.properties.PropertyMapping;
import org.springframework.test.web.reactive.server.WebTestClient;


@Target({ ElementType.TYPE, ElementType.METHOD })
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@ImportAutoConfiguration
@PropertyMapping("spring.test.webtestclient")
public @interface AutoConfigureWebTestClient {

	
	String timeout() default "";

}
