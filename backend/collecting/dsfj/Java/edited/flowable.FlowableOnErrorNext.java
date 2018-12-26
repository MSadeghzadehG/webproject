

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.exceptions.*;
import io.reactivex.functions.Function;
import io.reactivex.internal.subscriptions.SubscriptionArbiter;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableOnErrorNext<T> extends AbstractFlowableWithUpstream<T, T> {
    final Function<? super Throwable, ? extends Publisher<? extends T>> nextSupplier;
    final boolean allowFatal;

    public FlowableOnErrorNext(Flowable<T> source,
            Function<? super Throwable, ? extends Publisher<? extends T>> nextSupplier, boolean allowFatal) {
        super(source);
        this.nextSupplier = nextSupplier;
        this.allowFatal = allowFatal;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        OnErrorNextSubscriber<T> parent = new OnErrorNextSubscriber<T>(s, nextSupplier, allowFatal);
        s.onSubscribe(parent.arbiter);
        source.subscribe(parent);
    }

    static final class OnErrorNextSubscriber<T> implements FlowableSubscriber<T> {
        final Subscriber<? super T> actual;
        final Function<? super Throwable, ? extends Publisher<? extends T>> nextSupplier;
        final boolean allowFatal;
        final SubscriptionArbiter arbiter;

        boolean once;

        boolean done;

        OnErrorNextSubscriber(Subscriber<? super T> actual, Function<? super Throwable, ? extends Publisher<? extends T>> nextSupplier, boolean allowFatal) {
            this.actual = actual;
            this.nextSupplier = nextSupplier;
            this.allowFatal = allowFatal;
            this.arbiter = new SubscriptionArbiter();
        }

        @Override
        public void onSubscribe(Subscription s) {
            arbiter.setSubscription(s);
        }

        @Override
        public void onNext(T t) {
            if (done) {
                return;
            }
            actual.onNext(t);
            if (!once) {
                arbiter.produced(1L);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (once) {
                if (done) {
                    RxJavaPlugins.onError(t);
                    return;
                }
                actual.onError(t);
                return;
            }
            once = true;

            if (allowFatal && !(t instanceof Exception)) {
                actual.onError(t);
                return;
            }

            Publisher<? extends T> p;

            try {
                p = nextSupplier.apply(t);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                actual.onError(new CompositeException(t, e));
                return;
            }

            if (p == null) {
                NullPointerException npe = new NullPointerException("Publisher is null");
                npe.initCause(t);
                actual.onError(npe);
                return;
            }

            p.subscribe(this);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            once = true;
            actual.onComplete();
        }
    }
}
