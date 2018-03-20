

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.Flowable;

public final class FlowableFromPublisher<T> extends Flowable<T> {
    final Publisher<? extends T> publisher;

    public FlowableFromPublisher(Publisher<? extends T> publisher) {
        this.publisher = publisher;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        publisher.subscribe(s);
    }
}
