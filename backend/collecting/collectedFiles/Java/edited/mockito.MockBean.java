

package org.springframework.boot.test.mock.mockito;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.MockSettings;

import org.springframework.context.ApplicationContext;
import org.springframework.core.annotation.AliasFor;
import org.springframework.test.context.junit4.SpringRunner;


@Target({ ElementType.TYPE, ElementType.FIELD })
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Repeatable(MockBeans.class)
public @interface MockBean {

	
	String name() default "";

	
	@AliasFor("classes")
	Class<?>[] value() default {};

	
	@AliasFor("value")
	Class<?>[] classes() default {};

	
	Class<?>[] extraInterfaces() default {};

	
	Answers answer() default Answers.RETURNS_DEFAULTS;

	
	boolean serializable() default false;

	
	MockReset reset() default MockReset.AFTER;

}
