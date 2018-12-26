

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.Flowable;
import io.reactivex.internal.operators.flowable.FlowableTake.TakeSubscriber;


public final class FlowableTakePublisher<T> extends Flowable<T> {

    final Publisher<T> source;
    final long limit;
    public FlowableTakePublisher(Publisher<T> source, long limit) {
        this.source = source;
        this.limit = limit;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new TakeSubscriber<T>(s, limit));
    }
}
