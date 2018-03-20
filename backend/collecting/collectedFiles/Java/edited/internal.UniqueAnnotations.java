

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.BindingAnnotation;

import java.lang.annotation.Annotation;
import java.lang.annotation.Retention;
import java.util.concurrent.atomic.AtomicInteger;

import static java.lang.annotation.RetentionPolicy.RUNTIME;


public class UniqueAnnotations {
    private UniqueAnnotations() {
    }

    private static final AtomicInteger nextUniqueValue = new AtomicInteger(1);

    
    public static Annotation create() {
        return create(nextUniqueValue.getAndIncrement());
    }

    static Annotation create(final int value) {
        return new Internal() {
            @Override
            public int value() {
                return value;
            }

            @Override
            public Class<? extends Annotation> annotationType() {
                return Internal.class;
            }

            @Override
            public String toString() {
                return "@" + Internal.class.getName() + "(value=" + value + ")";
            }

            @Override
            public boolean equals(Object o) {
                return o instanceof Internal
                        && ((Internal) o).value() == value();
            }

            @Override
            public int hashCode() {
                return (127 * "value".hashCode()) ^ value;
            }
        };
    }

    @Retention(RUNTIME)
    @BindingAnnotation
    @interface Internal {
        int value();
    }
}
