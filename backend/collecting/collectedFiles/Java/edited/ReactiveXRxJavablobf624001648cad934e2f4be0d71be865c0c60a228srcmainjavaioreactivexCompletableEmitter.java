

package io.reactivex;

import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Cancellable;


public interface CompletableEmitter {

    
    void onComplete();

    
    void onError(@NonNull Throwable t);

    
    void setDisposable(@Nullable Disposable d);

    
    void setCancellable(@Nullable Cancellable c);

    
    boolean isDisposed();

    
    @Experimental
    boolean tryOnError(@NonNull Throwable t);
}
