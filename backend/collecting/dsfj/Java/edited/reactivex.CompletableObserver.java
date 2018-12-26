

package io.reactivex;

import io.reactivex.annotations.NonNull;
import io.reactivex.disposables.Disposable;


public interface CompletableObserver {
    
    void onSubscribe(@NonNull Disposable d);

    
    void onComplete();

    
    void onError(@NonNull Throwable e);
}
