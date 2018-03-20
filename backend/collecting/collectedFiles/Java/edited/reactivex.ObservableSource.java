
package io.reactivex;

import io.reactivex.annotations.*;


public interface ObservableSource<T> {

    
    void subscribe(@NonNull Observer<? super T> observer);
}
