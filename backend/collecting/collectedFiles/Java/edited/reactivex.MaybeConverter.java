

package io.reactivex;

import io.reactivex.annotations.*;


@Experimental
public interface MaybeConverter<T, R> {
    
    @NonNull
    R apply(@NonNull Maybe<T> upstream);
}
