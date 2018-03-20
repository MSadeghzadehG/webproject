

package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.AtomicLong;

import io.reactivex.internal.fuseable.QueueSubscription;


public abstract class BasicQueueSubscription<T> extends AtomicLong implements QueueSubscription<T> {


    private static final long serialVersionUID = -6671519529404341862L;

    @Override
    public final boolean offer(T e) {
        throw new UnsupportedOperationException("Should not be called!");
    }

    @Override
    public final boolean offer(T v1, T v2) {
        throw new UnsupportedOperationException("Should not be called!");
    }
}
