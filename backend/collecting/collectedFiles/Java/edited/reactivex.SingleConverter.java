

package io.reactivex;

import io.reactivex.annotations.*;


@Experimental
public interface SingleConverter<T, R> {
    
    @NonNull
    R apply(@NonNull Single<T> upstream);
}
