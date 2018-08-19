
package io.reactivex;

import io.reactivex.annotations.*;


public interface CompletableOnSubscribe {

    
    void subscribe(@NonNull CompletableEmitter emitter) throws Exception;
}

