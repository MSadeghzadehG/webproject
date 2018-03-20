
package io.reactivex;

import io.reactivex.annotations.*;


public interface MaybeOnSubscribe<T> {

    
    void subscribe(@NonNull MaybeEmitter<T> emitter) throws Exception;
}

