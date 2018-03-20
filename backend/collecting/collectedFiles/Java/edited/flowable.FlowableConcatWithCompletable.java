

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class FlowableConcatWithCompletable<T> extends AbstractFlowableWithUpstream<T, T> {

    final CompletableSource other;

    public FlowableConcatWithCompletable(Flowable<T> source, CompletableSource other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new ConcatWithSubscriber<T>(s, other));
    }

    static final class ConcatWithSubscriber<T>
    extends AtomicReference<Disposable>
    implements FlowableSubscriber<T>, CompletableObserver, Subscription {

        private static final long serialVersionUID = -7346385463600070225L;

        final Subscriber<? super T> actual;

        Subscription upstream;

        CompletableSource other;

        boolean inCompletable;

        ConcatWithSubscriber(Subscriber<? super T> actual, CompletableSource other) {
            this.actual = actual;
            this.other = other;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(upstream, s)) {
                this.upstream = s;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(this, d);
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (inCompletable) {
                actual.onComplete();
            } else {
                inCompletable = true;
                upstream = SubscriptionHelper.CANCELLED;
                CompletableSource cs = other;
                other = null;
                cs.subscribe(this);
            }
        }

        @Override
        public void request(long n) {
            upstream.request(n);
        }

        @Override
        public void cancel() {
            upstream.cancel();
            DisposableHelper.dispose(this);
        }
    }
}
