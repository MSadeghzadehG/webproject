
package io.reactivex;

import io.reactivex.annotations.*;


public interface SingleOnSubscribe<T> {

    
    void subscribe(@NonNull SingleEmitter<T> emitter) throws Exception;
}

