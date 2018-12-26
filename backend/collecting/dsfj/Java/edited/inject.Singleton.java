

package org.elasticsearch.common.inject;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.RetentionPolicy.RUNTIME;


@Target({ElementType.TYPE, ElementType.METHOD})
@Retention(RUNTIME)
@ScopeAnnotation
public @interface Singleton {
}
