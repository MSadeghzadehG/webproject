
package io.reactivex;

import io.reactivex.annotations.*;


public interface FlowableOnSubscribe<T> {

    
    void subscribe(@NonNull FlowableEmitter<T> emitter) throws Exception;
}

