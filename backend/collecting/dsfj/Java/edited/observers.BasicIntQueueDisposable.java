

package io.reactivex.internal.observers;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.internal.fuseable.QueueDisposable;


public abstract class BasicIntQueueDisposable<T>
extends AtomicInteger
implements QueueDisposable<T> {


    private static final long serialVersionUID = -1001730202384742097L;

    @Override
    public final boolean offer(T e) {
        throw new UnsupportedOperationException("Should not be called");
    }

    @Override
    public final boolean offer(T v1, T v2) {
        throw new UnsupportedOperationException("Should not be called");
    }
}
