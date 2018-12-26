

package org.elasticsearch.index.settings;

import org.elasticsearch.common.inject.BindingAnnotation;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.ElementType.PARAMETER;
import static java.lang.annotation.RetentionPolicy.RUNTIME;


@BindingAnnotation
@Target({FIELD, PARAMETER})
@Retention(RUNTIME)
@Documented
public @interface IndexDynamicSettings {

}
