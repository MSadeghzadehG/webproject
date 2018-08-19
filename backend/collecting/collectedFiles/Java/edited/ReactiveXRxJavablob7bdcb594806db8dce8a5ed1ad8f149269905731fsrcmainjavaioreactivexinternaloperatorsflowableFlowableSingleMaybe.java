

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscription;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.fuseable.FuseToFlowable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableSingleMaybe<T> extends Maybe<T> implements FuseToFlowable<T> {

    final Flowable<T> source;

    public FlowableSingleMaybe(Flowable<T> source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> s) {
        source.subscribe(new SingleElementSubscriber<T>(s));
    }

    @Override
    public Flowable<T> fuseToFlowable() {
        return RxJavaPlugins.onAssembly(new FlowableSingle<T>(source, null, false));
    }

    static final class SingleElementSubscriber<T>
    implements FlowableSubscriber<T>, Disposable {

        final MaybeObserver<? super T> actual;

        Subscription s;

        boolean done;

        T value;

        SingleElementSubscriber(MaybeObserver<? super T> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;
                actual.onSubscribe(this);
                s.request(Long.MAX_VALUE);
            }
        }

        @Override
        public void onNext(T t) {
            if (done) {
                return;
            }
            if (value != null) {
                done = true;
                s.cancel();
                s = SubscriptionHelper.CANCELLED;
                actual.onError(new IllegalArgumentException("Sequence contains more than one element!"));
                return;
            }
            value = t;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            s = SubscriptionHelper.CANCELLED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            s = SubscriptionHelper.CANCELLED;
            T v = value;
            value = null;
            if (v == null) {
                actual.onComplete();
            } else {
                actual.onSuccess(v);
            }
        }

        @Override
        public void dispose() {
            s.cancel();
            s = SubscriptionHelper.CANCELLED;
        }

        @Override
        public boolean isDisposed() {
            return s == SubscriptionHelper.CANCELLED;
        }
    }
}
