

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.schedulers.Timed;

public final class FlowableTimeInterval<T> extends AbstractFlowableWithUpstream<T, Timed<T>> {
    final Scheduler scheduler;
    final TimeUnit unit;

    public FlowableTimeInterval(Flowable<T> source, TimeUnit unit, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
        this.unit = unit;
    }


    @Override
    protected void subscribeActual(Subscriber<? super Timed<T>> s) {
        source.subscribe(new TimeIntervalSubscriber<T>(s, unit, scheduler));
    }

    static final class TimeIntervalSubscriber<T> implements FlowableSubscriber<T>, Subscription {
        final Subscriber<? super Timed<T>> actual;
        final TimeUnit unit;
        final Scheduler scheduler;

        Subscription s;

        long lastTime;

        TimeIntervalSubscriber(Subscriber<? super Timed<T>> actual, TimeUnit unit, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
            this.unit = unit;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                lastTime = scheduler.now(unit);
                this.s = s;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            long now = scheduler.now(unit);
            long last = lastTime;
            lastTime = now;
            long delta = now - last;
            actual.onNext(new Timed<T>(t, delta, unit));
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }

        @Override
        public void request(long n) {
            s.request(n);
        }

        @Override
        public void cancel() {
            s.cancel();
        }
    }
}
