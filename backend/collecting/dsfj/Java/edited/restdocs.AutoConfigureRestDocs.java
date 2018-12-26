

package org.springframework.boot.test.autoconfigure.restdocs;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.test.autoconfigure.properties.PropertyMapping;
import org.springframework.context.annotation.Import;
import org.springframework.core.annotation.AliasFor;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@ImportAutoConfiguration
@Import(RestDocumentationContextProviderRegistrar.class)
@PropertyMapping("spring.test.restdocs")
public @interface AutoConfigureRestDocs {

	
	@AliasFor("outputDir")
	String value() default "";

	
	@AliasFor("value")
	String outputDir() default "";

	
	String uriScheme() default "http";

	
	String uriHost() default "localhost";

	
	int uriPort() default 8080;

}
