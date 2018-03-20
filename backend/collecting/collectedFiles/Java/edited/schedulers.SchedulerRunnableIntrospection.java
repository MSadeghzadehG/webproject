

package io.reactivex.schedulers;

import io.reactivex.annotations.*;
import io.reactivex.functions.Function;
import io.reactivex.plugins.RxJavaPlugins;


@Experimental
public interface SchedulerRunnableIntrospection {

    
    @NonNull
    Runnable getWrappedRunnable();
}
