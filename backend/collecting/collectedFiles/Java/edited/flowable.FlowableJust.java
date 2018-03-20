

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.internal.fuseable.ScalarCallable;
import io.reactivex.internal.subscriptions.ScalarSubscription;


public final class FlowableJust<T> extends Flowable<T> implements ScalarCallable<T> {
    private final T value;
    public FlowableJust(final T value) {
        this.value = value;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        s.onSubscribe(new ScalarSubscription<T>(s, value));
    }

    @Override
    public T call() {
        return value;
    }
}
