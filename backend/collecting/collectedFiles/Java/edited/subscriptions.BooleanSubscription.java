
package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.AtomicBoolean;

import org.reactivestreams.Subscription;


public final class BooleanSubscription extends AtomicBoolean implements Subscription {

    private static final long serialVersionUID = -8127758972444290902L;

    @Override
    public void request(long n) {
        SubscriptionHelper.validate(n);
    }

    @Override
    public void cancel() {
        lazySet(true);
    }

    
    public boolean isCancelled() {
        return get();
    }

    @Override
    public String toString() {
        return "BooleanSubscription(cancelled=" + get() + ")";
    }
}
