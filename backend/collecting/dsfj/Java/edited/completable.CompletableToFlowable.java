

package io.reactivex.internal.operators.completable;

import org.reactivestreams.Subscriber;

import io.reactivex.*;
import io.reactivex.internal.observers.SubscriberCompletableObserver;

public final class CompletableToFlowable<T> extends Flowable<T> {

    final CompletableSource source;

    public CompletableToFlowable(CompletableSource source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        SubscriberCompletableObserver<T> os = new SubscriberCompletableObserver<T>(s);
        source.subscribe(os);
    }
}
