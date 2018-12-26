

package io.reactivex;

import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Cancellable;


public interface FlowableEmitter<T> extends Emitter<T> {

    
    void setDisposable(@Nullable Disposable s);

    
    void setCancellable(@Nullable Cancellable c);

    
    long requested();

    
    boolean isCancelled();

    
    @NonNull
    FlowableEmitter<T> serialize();

    
    @Experimental
    boolean tryOnError(@NonNull Throwable t);
}
