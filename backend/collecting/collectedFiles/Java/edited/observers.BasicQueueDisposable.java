

package io.reactivex.internal.observers;

import io.reactivex.internal.fuseable.QueueDisposable;


public abstract class BasicQueueDisposable<T> implements QueueDisposable<T> {

    @Override
    public final boolean offer(T e) {
        throw new UnsupportedOperationException("Should not be called");
    }

    @Override
    public final boolean offer(T v1, T v2) {
        throw new UnsupportedOperationException("Should not be called");
    }
}
