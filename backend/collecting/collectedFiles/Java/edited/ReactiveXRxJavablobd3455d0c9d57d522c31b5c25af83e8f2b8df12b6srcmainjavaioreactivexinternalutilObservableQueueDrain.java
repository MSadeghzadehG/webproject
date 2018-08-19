

package io.reactivex.internal.util;

import io.reactivex.Observer;

public interface ObservableQueueDrain<T, U> {

    boolean cancelled();

    boolean done();

    Throwable error();

    boolean enter();

    
    int leave(int m);

    
    void accept(Observer<? super U> a, T v);
}
