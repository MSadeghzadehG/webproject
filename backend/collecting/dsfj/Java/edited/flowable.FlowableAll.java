
package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableAll<T> extends AbstractFlowableWithUpstream<T, Boolean> {

    final Predicate<? super T> predicate;

    public FlowableAll(Flowable<T> source, Predicate<? super T> predicate) {
        super(source);
        this.predicate = predicate;
    }

    @Override
    protected void subscribeActual(Subscriber<? super Boolean> s) {
        source.subscribe(new AllSubscriber<T>(s, predicate));
    }

    static final class AllSubscriber<T> extends DeferredScalarSubscription<Boolean> implements FlowableSubscriber<T> {

        private static final long serialVersionUID = -3521127104134758517L;
        final Predicate<? super T> predicate;

        Subscription s;

        boolean done;

        AllSubscriber(Subscriber<? super Boolean> actual, Predicate<? super T> predicate) {
            super(actual);
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
                onError(e);
                return;
            }
            if (!b) {
                done = true;
                s.cancel();
                complete(false);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;

            complete(true);
        }

        @Override
        public void cancel() {
            super.cancel();
            s.cancel();
        }
    }
}
