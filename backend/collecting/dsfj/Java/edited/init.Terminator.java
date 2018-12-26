package hudson.init;

import org.jvnet.hudson.annotation_indexer.Indexed;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static hudson.init.TermMilestone.*;
import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.RetentionPolicy.RUNTIME;


@Indexed
@Documented
@Retention(RUNTIME)
@Target(METHOD)
public @interface Terminator {
    
    TermMilestone after() default STARTED;

    
    TermMilestone before() default COMPLETED;

    
    String[] requires() default {};

    
    String[] attains() default {};

    
    String displayName() default "";
}
