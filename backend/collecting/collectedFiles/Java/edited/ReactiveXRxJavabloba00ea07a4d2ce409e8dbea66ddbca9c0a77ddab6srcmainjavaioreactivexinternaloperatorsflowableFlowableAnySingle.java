
package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.fuseable.FuseToFlowable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableAnySingle<T> extends Single<Boolean> implements FuseToFlowable<Boolean> {
    final Flowable<T> source;

    final Predicate<? super T> predicate;

    public FlowableAnySingle(Flowable<T> source, Predicate<? super T> predicate) {
        this.source = source;
        this.predicate = predicate;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super Boolean> s) {
        source.subscribe(new AnySubscriber<T>(s, predicate));
    }

    @Override
    public Flowable<Boolean> fuseToFlowable() {
        return RxJavaPlugins.onAssembly(new FlowableAny<T>(source, predicate));
    }

    static final class AnySubscriber<T> implements FlowableSubscriber<T>, Disposable {

        final SingleObserver<? super Boolean> actual;

        final Predicate<? super T> predicate;

        Subscription s;

        boolean done;

        AnySubscriber(SingleObserver<? super Boolean> actual, Predicate<? super T> predicate) {
            this.actual = actual;
            this.predicate = predicate;
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
            boolean b;
            try {
                b = predicate.test(t);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                s.cancel();
                s = SubscriptionHelper.CANCELLED;
                onError(e);
                return;
            }
            if (b) {
                done = true;
                s.cancel();
                s = SubscriptionHelper.CANCELLED;
                actual.onSuccess(true);
            }
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
            if (!done) {
                done = true;
                s = SubscriptionHelper.CANCELLED;
                actual.onSuccess(false);
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
