

package org.springframework.boot.test.autoconfigure.web.reactive;

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
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Import;
import org.springframework.core.annotation.AliasFor;
import org.springframework.test.context.BootstrapWith;
import org.springframework.test.web.reactive.server.WebTestClient;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@BootstrapWith(WebFluxTestContextBootstrapper.class)
@OverrideAutoConfiguration(enabled = false)
@TypeExcludeFilters(WebFluxTypeExcludeFilter.class)
@AutoConfigureCache
@AutoConfigureWebFlux
@AutoConfigureWebTestClient
@ImportAutoConfiguration
public @interface WebFluxTest {

	
	@AliasFor("controllers")
	Class<?>[] value() default {};

	
	@AliasFor("value")
	Class<?>[] controllers() default {};

	
	boolean useDefaultFilters() default true;

	
	ComponentScan.Filter[] includeFilters() default {};

	
	ComponentScan.Filter[] excludeFilters() default {};

	
	@AliasFor(annotation = ImportAutoConfiguration.class, attribute = "exclude")
	Class<?>[] excludeAutoConfiguration() default {};

}
