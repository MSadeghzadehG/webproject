
package io.reactivex;

import io.reactivex.annotations.*;


public interface CompletableSource {

    
    void subscribe(@NonNull CompletableObserver cs);
}
