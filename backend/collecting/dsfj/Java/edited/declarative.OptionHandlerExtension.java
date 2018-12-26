

package hudson.cli.declarative;

import org.jvnet.hudson.annotation_indexer.Indexed;
import org.kohsuke.args4j.spi.OptionHandler;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.TYPE;
import static java.lang.annotation.RetentionPolicy.RUNTIME;


@Indexed
@Retention(RUNTIME)
@Target({TYPE})
@Documented
public @interface OptionHandlerExtension {
}
