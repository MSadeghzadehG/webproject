

package org.springframework.boot.test.mock.mockito;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.junit.runner.RunWith;
import org.mockito.Mockito;

import org.springframework.context.ApplicationContext;
import org.springframework.core.annotation.AliasFor;
import org.springframework.test.context.junit4.SpringRunner;


@Target({ ElementType.TYPE, ElementType.FIELD })
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Repeatable(SpyBeans.class)
public @interface SpyBean {

	
	String name() default "";

	
	@AliasFor("classes")
	Class<?>[] value() default {};

	
	@AliasFor("value")
	Class<?>[] classes() default {};

	
	MockReset reset() default MockReset.AFTER;

	
	boolean proxyTargetAware() default true;

}
