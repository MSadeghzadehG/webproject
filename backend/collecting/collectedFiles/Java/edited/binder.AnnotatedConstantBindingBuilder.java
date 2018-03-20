

package org.elasticsearch.common.inject.binder;

import java.lang.annotation.Annotation;


public interface AnnotatedConstantBindingBuilder {

    
    ConstantBindingBuilder annotatedWith(
            Class<? extends Annotation> annotationType);

    
    ConstantBindingBuilder annotatedWith(Annotation annotation);
}
