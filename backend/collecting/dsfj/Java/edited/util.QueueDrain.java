

package io.reactivex.internal.util;

import org.reactivestreams.Subscriber;

public interface QueueDrain<T, U> {

    boolean cancelled();

    boolean done();

    Throwable error();

    boolean enter();

    long requested();

    long produced(long n);

    
    int leave(int m);

    
    boolean accept(Subscriber<? super U> a, T v);
}
