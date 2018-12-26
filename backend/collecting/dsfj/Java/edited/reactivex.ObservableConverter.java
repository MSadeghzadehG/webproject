

package io.reactivex;

import io.reactivex.annotations.*;


@Experimental
public interface ObservableConverter<T, R> {
    
    @NonNull
    R apply(@NonNull Observable<T> upstream);
}
