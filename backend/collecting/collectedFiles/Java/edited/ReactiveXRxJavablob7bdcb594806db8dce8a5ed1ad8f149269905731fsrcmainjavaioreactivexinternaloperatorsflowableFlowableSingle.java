

package io.reactivex.internal.operators.flowable;

import java.util.NoSuchElementException;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableSingle<T> extends AbstractFlowableWithUpstream<T, T> {

    final T defaultValue;

    final boolean failOnEmpty;

    public FlowableSingle(Flowable<T> source, T defaultValue, boolean failOnEmpty) {
        super(source);
        this.defaultValue = defaultValue;
        this.failOnEmpty = failOnEmpty;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new SingleElementSubscriber<T>(s, defaultValue, failOnEmpty));
    }

    static final class SingleElementSubscriber<T> extends DeferredScalarSubscription<T>
    implements FlowableSubscriber<T> {

        private static final long serialVersionUID = -5526049321428043809L;

        final T defaultValue;

        final boolean failOnEmpty;

        Subscription s;

        boolean done;

        SingleElementSubscriber(Subscriber<? super T> actual, T defaultValue, boolean failOnEmpty) {
            super(actual);
            this.defaultValue = defaultValue;
            this.failOnEmpty = failOnEmpty;
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
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            T v = value;
            value = null;
            if (v == null) {
                v = defaultValue;
            }
            if (v == null) {
                if (failOnEmpty) {
                    actual.onError(new NoSuchElementException());
                } else {
                    actual.onComplete();
                }
            } else {
                complete(v);
            }
        }

        @Override
        public void cancel() {
            super.cancel();
            s.cancel();
        }
    }
}
