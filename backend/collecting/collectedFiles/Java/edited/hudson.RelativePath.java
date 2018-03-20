package hudson;

import org.kohsuke.stapler.QueryParameter;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.*;
import static java.lang.annotation.RetentionPolicy.*;


@Documented
@Target(PARAMETER)
@Retention(RUNTIME)
public @interface RelativePath {
    String value();
}
