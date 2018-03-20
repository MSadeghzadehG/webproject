

package org.elasticsearch.common.inject.binder;

import java.lang.annotation.Annotation;


public interface AnnotatedElementBuilder {

    
    void annotatedWith(Class<? extends Annotation> annotationType);

    
    void annotatedWith(Annotation annotation);
}
