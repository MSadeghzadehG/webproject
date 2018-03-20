

package io.reactivex;

import io.reactivex.annotations.*;


@Experimental
public interface CompletableConverter<R> {
    
    @NonNull
    R apply(@NonNull Completable upstream);
}
