

package org.springframework.boot.autoconfigure.domain;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.context.annotation.Import;
import org.springframework.core.annotation.AliasFor;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Import(EntityScanPackages.Registrar.class)
public @interface EntityScan {

	
	@AliasFor("basePackages")
	String[] value() default {};

	
	@AliasFor("value")
	String[] basePackages() default {};

	
	Class<?>[] basePackageClasses() default {};

}
