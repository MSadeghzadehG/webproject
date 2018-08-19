
package io.reactivex;

import io.reactivex.annotations.*;


public interface ObservableOnSubscribe<T> {

    
    void subscribe(@NonNull ObservableEmitter<T> emitter) throws Exception;
}

