

package org.springframework.boot.test.autoconfigure.json;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.test.autoconfigure.properties.PropertyMapping;
import org.springframework.boot.test.json.BasicJsonTester;
import org.springframework.boot.test.json.GsonTester;
import org.springframework.boot.test.json.JacksonTester;
import org.springframework.boot.test.json.JsonbTester;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@ImportAutoConfiguration
@PropertyMapping("spring.test.jsontesters")
public @interface AutoConfigureJsonTesters {

	
	boolean enabled() default true;

}
