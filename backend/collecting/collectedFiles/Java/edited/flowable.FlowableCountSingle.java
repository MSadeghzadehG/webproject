

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.fuseable.FuseToFlowable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableCountSingle<T> extends Single<Long> implements FuseToFlowable<Long> {

    final Flowable<T> source;

    public FlowableCountSingle(Flowable<T> source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super Long> s) {
        source.subscribe(new CountSubscriber(s));
    }

    @Override
    public Flowable<Long> fuseToFlowable() {
        return RxJavaPlugins.onAssembly(new FlowableCount<T>(source));
    }

    static final class CountSubscriber implements FlowableSubscriber<Object>, Disposable {

        final SingleObserver<? super Long> actual;

        Subscription s;

        long count;

        CountSubscriber(SingleObserver<? super Long> actual) {
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
        public void onNext(Object t) {
            count++;
        }

        @Override
        public void onError(Throwable t) {
            s = SubscriptionHelper.CANCELLED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            s = SubscriptionHelper.CANCELLED;
            actual.onSuccess(count);
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
