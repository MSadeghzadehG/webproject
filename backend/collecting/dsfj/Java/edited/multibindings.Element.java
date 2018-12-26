


package org.elasticsearch.common.inject.multibindings;

import org.elasticsearch.common.inject.BindingAnnotation;

import java.lang.annotation.Retention;

import static java.lang.annotation.RetentionPolicy.RUNTIME;


@Retention(RUNTIME)
@BindingAnnotation
@interface Element {
    String setName();

    int uniqueId();
}
