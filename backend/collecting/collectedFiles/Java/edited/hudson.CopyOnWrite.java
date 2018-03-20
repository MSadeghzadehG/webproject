
package hudson;

import java.lang.annotation.Documented;
import static java.lang.annotation.ElementType.FIELD;
import java.lang.annotation.Retention;
import static java.lang.annotation.RetentionPolicy.SOURCE;
import java.lang.annotation.Target;


@Retention(SOURCE)
@Documented
@Target(FIELD)
public @interface CopyOnWrite {
}
