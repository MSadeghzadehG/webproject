

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.internal.fuseable.ScalarCallable;
import io.reactivex.internal.subscriptions.EmptySubscription;


public final class FlowableEmpty extends Flowable<Object> implements ScalarCallable<Object> {

    public static final Flowable<Object> INSTANCE = new FlowableEmpty();

    private FlowableEmpty() {
    }

    @Override
    public void subscribeActual(Subscriber<? super Object> s) {
        EmptySubscription.complete(s);
    }

    @Override
    public Object call() {
        return null;     }
}
