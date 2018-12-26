

package org.springframework.boot.web.servlet;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import javax.servlet.annotation.WebFilter;
import javax.servlet.annotation.WebListener;
import javax.servlet.annotation.WebServlet;

import org.springframework.context.annotation.Import;
import org.springframework.core.annotation.AliasFor;


@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Import(ServletComponentScanRegistrar.class)
public @interface ServletComponentScan {

	
	@AliasFor("basePackages")
	String[] value() default {};

	
	@AliasFor("value")
	String[] basePackages() default {};

	
	Class<?>[] basePackageClasses() default {};

}
