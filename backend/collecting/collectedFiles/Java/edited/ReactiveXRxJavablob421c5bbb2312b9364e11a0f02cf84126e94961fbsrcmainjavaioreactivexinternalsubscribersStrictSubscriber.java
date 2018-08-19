

package io.reactivex.internal.subscribers;

import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.FlowableSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.*;


public class StrictSubscriber<T>
extends AtomicInteger
implements FlowableSubscriber<T>, Subscription {

    private static final long serialVersionUID = -4945028590049415624L;

    final Subscriber<? super T> actual;

    final AtomicThrowable error;

    final AtomicLong requested;

    final AtomicReference<Subscription> s;

    final AtomicBoolean once;

    volatile boolean done;

    public StrictSubscriber(Subscriber<? super T> actual) {
        this.actual = actual;
        this.error = new AtomicThrowable();
        this.requested = new AtomicLong();
        this.s = new AtomicReference<Subscription>();
        this.once = new AtomicBoolean();
    }

    @Override
    public void request(long n) {
        if (n <= 0) {
            cancel();
            onError(new IllegalArgumentException("§3.9 violated: positive request amount required but it was " + n));
        } else {
            SubscriptionHelper.deferredRequest(s, requested, n);
        }
    }

    @Override
    public void cancel() {
        if (!done) {
            SubscriptionHelper.cancel(s);
        }
    }

    @Override
    public void onSubscribe(Subscription s) {
        if (once.compareAndSet(false, true)) {

            actual.onSubscribe(this);

            SubscriptionHelper.deferredSetOnce(this.s, requested, s);
        } else {
            s.cancel();
            cancel();
            onError(new IllegalStateException("§2.12 violated: onSubscribe must be called at most once"));
        }
    }

    @Override
    public void onNext(T t) {
        HalfSerializer.onNext(actual, t, this, error);
    }

    @Override
    public void onError(Throwable t) {
        done = true;
        HalfSerializer.onError(actual, t, this, error);
    }

    @Override
    public void onComplete() {
        done = true;
        HalfSerializer.onComplete(actual, this, error);
    }
}
