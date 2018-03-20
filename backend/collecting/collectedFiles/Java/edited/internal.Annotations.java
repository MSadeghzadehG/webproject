

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.Classes;
import org.elasticsearch.common.inject.BindingAnnotation;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.ScopeAnnotation;
import org.elasticsearch.common.inject.TypeLiteral;

import java.lang.annotation.Annotation;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Member;


public class Annotations {

    
    public static boolean isRetainedAtRuntime(Class<? extends Annotation> annotationType) {
        Retention retention = annotationType.getAnnotation(Retention.class);
        return retention != null && retention.value() == RetentionPolicy.RUNTIME;
    }

    
    public static Class<? extends Annotation> findScopeAnnotation(
            Errors errors, Class<?> implementation) {
        return findScopeAnnotation(errors, implementation.getAnnotations());
    }

    
    public static Class<? extends Annotation> findScopeAnnotation(Errors errors, Annotation[] annotations) {
        Class<? extends Annotation> found = null;

        for (Annotation annotation : annotations) {
            if (annotation.annotationType().getAnnotation(ScopeAnnotation.class) != null) {
                if (found != null) {
                    errors.duplicateScopeAnnotations(found, annotation.annotationType());
                } else {
                    found = annotation.annotationType();
                }
            }
        }

        return found;
    }

    public static boolean isScopeAnnotation(Class<? extends Annotation> annotationType) {
        return annotationType.getAnnotation(ScopeAnnotation.class) != null;
    }

    
    public static void checkForMisplacedScopeAnnotations(
            Class<?> type, Object source, Errors errors) {
        if (Classes.isConcrete(type)) {
            return;
        }

        Class<? extends Annotation> scopeAnnotation = findScopeAnnotation(errors, type);
        if (scopeAnnotation != null) {
            errors.withSource(type).scopeAnnotationOnAbstractType(scopeAnnotation, type, source);
        }
    }

    
    public static Key<?> getKey(TypeLiteral<?> type, Member member, Annotation[] annotations,
                                Errors errors) throws ErrorsException {
        int numErrorsBefore = errors.size();
        Annotation found = findBindingAnnotation(errors, member, annotations);
        errors.throwIfNewErrors(numErrorsBefore);
        return found == null ? Key.get(type) : Key.get(type, found);
    }

    
    public static Annotation findBindingAnnotation(
            Errors errors, Member member, Annotation[] annotations) {
        Annotation found = null;

        for (Annotation annotation : annotations) {
            if (annotation.annotationType().getAnnotation(BindingAnnotation.class) != null) {
                if (found != null) {
                    errors.duplicateBindingAnnotations(member,
                            found.annotationType(), annotation.annotationType());
                } else {
                    found = annotation;
                }
            }
        }

        return found;
    }
}
