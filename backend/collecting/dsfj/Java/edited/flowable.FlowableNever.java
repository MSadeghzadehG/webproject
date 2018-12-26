
package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.internal.subscriptions.EmptySubscription;

public final class FlowableNever extends Flowable<Object> {
    public static final Flowable<Object> INSTANCE = new FlowableNever();

    private FlowableNever() {
    }

    @Override
    public void subscribeActual(Subscriber<? super Object> s) {
        s.onSubscribe(EmptySubscription.INSTANCE);
    }
}
