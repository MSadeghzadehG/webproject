

package org.springframework.boot.test.autoconfigure.properties;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.context.annotation.PropertySource;
import org.springframework.test.context.TestPropertySource;


@Retention(RetentionPolicy.RUNTIME)
@Target({ ElementType.TYPE, ElementType.METHOD })
@Documented
public @interface PropertyMapping {

	
	String value() default "";

	
	SkipPropertyMapping skip() default SkipPropertyMapping.NO;

}
