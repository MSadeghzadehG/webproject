

package io.reactivex.internal.fuseable;

import io.reactivex.annotations.*;


public interface SimpleQueue<T> {

    
    boolean offer(@NonNull T value);

    
    boolean offer(@NonNull T v1, @NonNull T v2);

    
    @Nullable
    T poll() throws Exception;

    
    boolean isEmpty();

    
    void clear();
}
