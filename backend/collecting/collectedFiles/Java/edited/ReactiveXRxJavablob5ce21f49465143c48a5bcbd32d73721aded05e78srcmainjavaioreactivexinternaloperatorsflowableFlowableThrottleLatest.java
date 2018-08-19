

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.annotations.Experimental;
import io.reactivex.exceptions.MissingBackpressureException;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.BackpressureHelper;


@Experimental
public final class FlowableThrottleLatest<T> extends AbstractFlowableWithUpstream<T, T> {

    final long timeout;

    final TimeUnit unit;

    final Scheduler scheduler;

    final boolean emitLast;

    public FlowableThrottleLatest(Flowable<T> source,
            long timeout, TimeUnit unit, Scheduler scheduler,
            boolean emitLast) {
        super(source);
        this.timeout = timeout;
        this.unit = unit;
        this.scheduler = scheduler;
        this.emitLast = emitLast;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new ThrottleLatestSubscriber<T>(s, timeout, unit, scheduler.createWorker(), emitLast));
    }

    static final class ThrottleLatestSubscriber<T>
    extends AtomicInteger
    implements FlowableSubscriber<T>, Subscription, Runnable {

        private static final long serialVersionUID = -8296689127439125014L;

        final Subscriber<? super T> downstream;

        final long timeout;

        final TimeUnit unit;

        final Scheduler.Worker worker;

        final boolean emitLast;

        final AtomicReference<T> latest;

        final AtomicLong requested;

        Subscription upstream;

        volatile boolean done;
        Throwable error;

        volatile boolean cancelled;

        volatile boolean timerFired;

        long emitted;

        boolean timerRunning;

        ThrottleLatestSubscriber(Subscriber<? super T> downstream,
                long timeout, TimeUnit unit, Scheduler.Worker worker,
                boolean emitLast) {
            this.downstream = downstream;
            this.timeout = timeout;
            this.unit = unit;
            this.worker = worker;
            this.emitLast = emitLast;
            this.latest = new AtomicReference<T>();
            this.requested = new AtomicLong();
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(upstream, s)) {
                upstream = s;
                downstream.onSubscribe(this);
                s.request(Long.MAX_VALUE);
            }
        }

        @Override
        public void onNext(T t) {
            latest.set(t);
            drain();
        }

        @Override
        public void onError(Throwable t) {
            error = t;
            done = true;
            drain();
        }

        @Override
        public void onComplete() {
            done = true;
            drain();
        }

        @Override
        public void request(long n) {
            if (SubscriptionHelper.validate(n)) {
                BackpressureHelper.add(requested, n);
            }
        }

        @Override
        public void cancel() {
            cancelled = true;
            upstream.cancel();
            worker.dispose();
            if (getAndIncrement() == 0) {
                latest.lazySet(null);
            }
        }

        @Override
        public void run() {
            timerFired = true;
            drain();
        }

        void drain() {
            if (getAndIncrement() != 0) {
                return;
            }

            int missed = 1;

            AtomicReference<T> latest = this.latest;
            AtomicLong requested = this.requested;
            Subscriber<? super T> downstream = this.downstream;

            for (;;) {

                for (;;) {
                    if (cancelled) {
                        latest.lazySet(null);
                        return;
                    }

                    boolean d = done;

                    if (d && error != null) {
                        latest.lazySet(null);
                        downstream.onError(error);
                        worker.dispose();
                        return;
                    }

                    T v = latest.get();
                    boolean empty = v == null;

                    if (d) {
                        if (!empty && emitLast) {
                            v = latest.getAndSet(null);
                            long e = emitted;
                            if (e != requested.get()) {
                                emitted = e + 1;
                                downstream.onNext(v);
                                downstream.onComplete();
                            } else {
                                downstream.onError(new MissingBackpressureException(
                                        "Could not emit final value due to lack of requests"));
                            }
                        } else {
                            latest.lazySet(null);
                            downstream.onComplete();
                        }
                        worker.dispose();
                        return;
                    }

                    if (empty) {
                        if (timerFired) {
                            timerRunning = false;
                            timerFired = false;
                        }
                        break;
                    }

                    if (!timerRunning || timerFired) {
                        v = latest.getAndSet(null);
                        long e = emitted;
                        if (e != requested.get()) {
                            downstream.onNext(v);
                            emitted = e + 1;
                        } else {
                            upstream.cancel();
                            downstream.onError(new MissingBackpressureException(
                                    "Could not emit value due to lack of requests"));
                            worker.dispose();
                            return;
                        }

                        timerFired = false;
                        timerRunning = true;
                        worker.schedule(this, timeout, unit);
                    } else {
                        break;
                    }
                }

                missed = addAndGet(-missed);
                if (missed == 0) {
                    break;
                }
            }
        }
    }
}
