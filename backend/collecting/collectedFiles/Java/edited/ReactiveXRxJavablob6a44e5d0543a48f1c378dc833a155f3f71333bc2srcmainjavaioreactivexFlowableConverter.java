

package io.reactivex;

import io.reactivex.annotations.*;


@Experimental
public interface FlowableConverter<T, R> {
    
    @NonNull
    R apply(@NonNull Flowable<T> upstream);
}
