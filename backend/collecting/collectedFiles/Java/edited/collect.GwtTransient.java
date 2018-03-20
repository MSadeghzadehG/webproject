

package com.google.common.collect;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.RetentionPolicy.RUNTIME;

import com.google.common.annotations.GwtCompatible;
import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;


@Documented
@GwtCompatible
@Retention(RUNTIME)
@Target(FIELD)
@interface GwtTransient {}
