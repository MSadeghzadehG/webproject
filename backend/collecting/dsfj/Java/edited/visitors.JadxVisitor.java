package jadx.core.dex.visitors;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;


@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface JadxVisitor {
	
	String name();

	
	String desc() default "";

	
	Class<? extends IDexTreeVisitor>[] runAfter() default {};

	
	Class<? extends IDexTreeVisitor>[] runBefore() default {};
}
