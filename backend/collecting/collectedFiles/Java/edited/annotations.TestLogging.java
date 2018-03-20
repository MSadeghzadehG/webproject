
package org.elasticsearch.test.junit.annotations;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.ElementType.PACKAGE;
import static java.lang.annotation.ElementType.TYPE;


@Retention(RetentionPolicy.RUNTIME)
@Target({PACKAGE, TYPE, METHOD})
public @interface TestLogging {
    String value();
}
