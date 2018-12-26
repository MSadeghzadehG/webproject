

package io.reactivex;

import io.reactivex.annotations.NonNull;
import io.reactivex.disposables.Disposable;


public interface Observer<T> {

    
    void onSubscribe(@NonNull Disposable d);

    
    void onNext(@NonNull T t);

    
    void onError(@NonNull Throwable e);

    
    void onComplete();

}
