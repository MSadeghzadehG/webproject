

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BiFunction;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class FlowableReduce<T> extends AbstractFlowableWithUpstream<T, T> {

    final BiFunction<T, T, T> reducer;

    public FlowableReduce(Flowable<T> source, BiFunction<T, T, T> reducer) {
        super(source);
        this.reducer = reducer;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new ReduceSubscriber<T>(s, reducer));
    }

    static final class ReduceSubscriber<T> extends DeferredScalarSubscription<T> implements FlowableSubscriber<T> {

        private static final long serialVersionUID = -4663883003264602070L;

        final BiFunction<T, T, T> reducer;

        Subscription s;

        ReduceSubscriber(Subscriber<? super T> actual, BiFunction<T, T, T> reducer) {
            super(actual);
            this.reducer = reducer;
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
            if (s == SubscriptionHelper.CANCELLED) {
                return;
            }

            T v = value;
            if (v == null) {
                value = t;
            } else {
                try {
                    value = ObjectHelper.requireNonNull(reducer.apply(v, t), "The reducer returned a null value");
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    s.cancel();
                    onError(ex);
                }
            }
        }

        @Override
        public void onError(Throwable t) {
            if (s == SubscriptionHelper.CANCELLED) {
                RxJavaPlugins.onError(t);
                return;
            }
            s = SubscriptionHelper.CANCELLED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (s == SubscriptionHelper.CANCELLED) {
                return;
            }
            s = SubscriptionHelper.CANCELLED;

            T v = value;
            if (v != null) {
                complete(v);
            } else {
                actual.onComplete();
            }
        }

        @Override
        public void cancel() {
            super.cancel();
            s.cancel();
            s = SubscriptionHelper.CANCELLED;
        }

    }
}
