
package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.subscribers.SerializedSubscriber;

public final class FlowableSerialized<T> extends AbstractFlowableWithUpstream<T, T> {
    public FlowableSerialized(Flowable<T> source) {
        super(source);
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new SerializedSubscriber<T>(s));
    }
}
