

package org.springframework.boot.test.autoconfigure.jdbc;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import javax.sql.DataSource;

import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.jdbc.EmbeddedDatabaseConnection;
import org.springframework.boot.test.autoconfigure.properties.PropertyMapping;
import org.springframework.boot.test.autoconfigure.properties.SkipPropertyMapping;


@Target({ ElementType.TYPE, ElementType.METHOD })
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@ImportAutoConfiguration
@PropertyMapping("spring.test.database")
public @interface AutoConfigureTestDatabase {

	
	@PropertyMapping(skip = SkipPropertyMapping.ON_DEFAULT_VALUE)
	Replace replace() default Replace.ANY;

	
	EmbeddedDatabaseConnection connection() default EmbeddedDatabaseConnection.NONE;

	
	enum Replace {

		
		ANY,

		
		AUTO_CONFIGURED,

		
		NONE

	}

}
