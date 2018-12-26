

package org.springframework.boot.test.autoconfigure.jdbc;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.test.autoconfigure.OverrideAutoConfiguration;
import org.springframework.boot.test.autoconfigure.core.AutoConfigureCache;
import org.springframework.boot.test.autoconfigure.filter.TypeExcludeFilters;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.SpringBootTestContextBootstrapper;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.core.annotation.AliasFor;
import org.springframework.test.context.BootstrapWith;
import org.springframework.transaction.annotation.Transactional;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@BootstrapWith(SpringBootTestContextBootstrapper.class)
@OverrideAutoConfiguration(enabled = false)
@TypeExcludeFilters(JdbcTypeExcludeFilter.class)
@Transactional
@AutoConfigureCache
@AutoConfigureJdbc
@AutoConfigureTestDatabase
@ImportAutoConfiguration
public @interface JdbcTest {

	
	boolean useDefaultFilters() default true;

	
	ComponentScan.Filter[] includeFilters() default {};

	
	ComponentScan.Filter[] excludeFilters() default {};

	
	@AliasFor(annotation = ImportAutoConfiguration.class, attribute = "exclude")
	Class<?>[] excludeAutoConfiguration() default {};

}
