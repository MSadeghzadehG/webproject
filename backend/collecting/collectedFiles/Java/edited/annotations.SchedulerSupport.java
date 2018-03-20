

package io.reactivex.annotations;

import java.lang.annotation.*;

import io.reactivex.schedulers.Schedulers;


@Retention(RetentionPolicy.RUNTIME)
@Documented
@Target({ElementType.METHOD, ElementType.TYPE})
public @interface SchedulerSupport {
    
    String NONE = "none";
    
    String CUSTOM = "custom";

        
    String COMPUTATION = "io.reactivex:computation";
    
    String IO = "io.reactivex:io";
    
    String NEW_THREAD = "io.reactivex:new-thread";
    
    String TRAMPOLINE = "io.reactivex:trampoline";
    
    @Experimental
    String SINGLE = "io.reactivex:single";

    
    String value();
}
