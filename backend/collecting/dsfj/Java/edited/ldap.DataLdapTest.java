

package org.springframework.boot.test.autoconfigure.data.ldap;

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
import org.springframework.boot.test.context.SpringBootTestContextBootstrapper;
import org.springframework.context.annotation.ComponentScan.Filter;
import org.springframework.core.annotation.AliasFor;
import org.springframework.test.context.BootstrapWith;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@BootstrapWith(SpringBootTestContextBootstrapper.class)
@OverrideAutoConfiguration(enabled = false)
@TypeExcludeFilters(DataLdapTypeExcludeFilter.class)
@AutoConfigureCache
@AutoConfigureDataLdap
@ImportAutoConfiguration
public @interface DataLdapTest {

	
	boolean useDefaultFilters() default true;

	
	Filter[] includeFilters() default {};

	
	Filter[] excludeFilters() default {};

	
	@AliasFor(annotation = ImportAutoConfiguration.class, attribute = "exclude")
	Class<?>[] excludeAutoConfiguration() default {};

}
