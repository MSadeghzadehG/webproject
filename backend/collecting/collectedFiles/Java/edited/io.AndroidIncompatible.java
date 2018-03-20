

package com.google.common.io;

import static java.lang.annotation.ElementType.ANNOTATION_TYPE;
import static java.lang.annotation.ElementType.CONSTRUCTOR;
import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.ElementType.METHOD;
import static java.lang.annotation.ElementType.TYPE;
import static java.lang.annotation.RetentionPolicy.CLASS;

import com.google.common.annotations.GwtCompatible;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;


@Retention(CLASS)
@Target({ANNOTATION_TYPE, CONSTRUCTOR, FIELD, METHOD, TYPE})
@GwtCompatible
@interface AndroidIncompatible {}
