

package org.elasticsearch.common.inject.binder;

import java.lang.annotation.Annotation;


public interface AnnotatedBindingBuilder<T> extends LinkedBindingBuilder<T> {

    
    LinkedBindingBuilder<T> annotatedWith(
            Class<? extends Annotation> annotationType);

    
    LinkedBindingBuilder<T> annotatedWith(Annotation annotation);
}
