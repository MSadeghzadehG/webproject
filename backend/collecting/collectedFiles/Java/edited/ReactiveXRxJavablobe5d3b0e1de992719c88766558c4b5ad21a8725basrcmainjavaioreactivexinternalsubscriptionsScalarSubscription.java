

package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.annotations.Nullable;
import org.reactivestreams.Subscriber;

import io.reactivex.internal.fuseable.QueueSubscription;


public final class ScalarSubscription<T> extends AtomicInteger implements QueueSubscription<T> {

    private static final long serialVersionUID = -3830916580126663321L;
    
    final T value;
    
    final Subscriber<? super T> subscriber;

    
    static final int NO_REQUEST = 0;
    
    static final int REQUESTED = 1;
    
    static final int CANCELLED = 2;

    public ScalarSubscription(Subscriber<? super T> subscriber, T value) {
        this.subscriber = subscriber;
        this.value = value;
    }

    @Override
    public void request(long n) {
        if (!SubscriptionHelper.validate(n)) {
            return;
        }
        if (compareAndSet(NO_REQUEST, REQUESTED)) {
            Subscriber<? super T> s = subscriber;

            s.onNext(value);
            if (get() != CANCELLED) {
                s.onComplete();
            }
        }

    }

    @Override
    public void cancel() {
        lazySet(CANCELLED);
    }

    
    public boolean isCancelled() {
        return get() == CANCELLED;
    }

    @Override
    public boolean offer(T e) {
        throw new UnsupportedOperationException("Should not be called!");
    }

    @Override
    public boolean offer(T v1, T v2) {
        throw new UnsupportedOperationException("Should not be called!");
    }

    @Nullable
    @Override
    public T poll() {
        if (get() == NO_REQUEST) {
            lazySet(REQUESTED);
            return value;
        }
        return null;
    }

    @Override
    public boolean isEmpty() {
        return get() != NO_REQUEST;
    }

    @Override
    public void clear() {
        lazySet(1);
    }

    @Override
    public int requestFusion(int mode) {
        return mode & SYNC;
    }
}
