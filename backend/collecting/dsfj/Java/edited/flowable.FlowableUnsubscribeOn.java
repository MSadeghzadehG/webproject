

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicBoolean;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableUnsubscribeOn<T> extends AbstractFlowableWithUpstream<T, T> {
    final Scheduler scheduler;
    public FlowableUnsubscribeOn(Flowable<T> source, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new UnsubscribeSubscriber<T>(s, scheduler));
    }

    static final class UnsubscribeSubscriber<T> extends AtomicBoolean implements FlowableSubscriber<T>, Subscription {

        private static final long serialVersionUID = 1015244841293359600L;

        final Subscriber<? super T> actual;
        final Scheduler scheduler;

        Subscription s;

        UnsubscribeSubscriber(Subscriber<? super T> actual, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            if (!get()) {
                actual.onNext(t);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (get()) {
                RxJavaPlugins.onError(t);
                return;
            }
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (!get()) {
                actual.onComplete();
            }
        }

        @Override
        public void request(long n) {
            s.request(n);
        }

        @Override
        public void cancel() {
            if (compareAndSet(false, true)) {
                scheduler.scheduleDirect(new Cancellation());
            }
        }

        final class Cancellation implements Runnable {
            @Override
            public void run() {
                s.cancel();
            }
        }
    }
}
