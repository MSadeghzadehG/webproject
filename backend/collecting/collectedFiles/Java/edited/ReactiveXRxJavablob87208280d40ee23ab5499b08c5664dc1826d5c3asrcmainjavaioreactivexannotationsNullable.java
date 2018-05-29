

package io.reactivex.annotations;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.ElementType.LOCAL_VARIABLE;
import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.ElementType.PARAMETER;
import static java.lang.annotation.RetentionPolicy.CLASS;


@Documented
@Target(value = {FIELD, METHOD, PARAMETER, LOCAL_VARIABLE})
@Retention(value = CLASS)
public @interface Nullable { }

