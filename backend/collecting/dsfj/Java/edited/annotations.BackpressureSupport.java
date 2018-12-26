

package io.reactivex.annotations;

import java.lang.annotation.*;


@Retention(RetentionPolicy.RUNTIME)
@Documented
@Target({ElementType.METHOD, ElementType.TYPE})
public @interface BackpressureSupport {
    
    BackpressureKind value();
}
