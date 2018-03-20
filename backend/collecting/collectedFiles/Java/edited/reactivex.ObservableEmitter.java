

package io.reactivex;

import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Cancellable;


public interface ObservableEmitter<T> extends Emitter<T> {

    
    void setDisposable(@Nullable Disposable d);

    
    void setCancellable(@Nullable Cancellable c);

    
    boolean isDisposed();

    
    @NonNull
    ObservableEmitter<T> serialize();

    
    @Experimental
    boolean tryOnError(@NonNull Throwable t);
}
