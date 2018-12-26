

package io.reactivex.internal.operators.flowable;

import java.util.ArrayDeque;
import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.BackpressureHelper;

public final class FlowableTakeLast<T> extends AbstractFlowableWithUpstream<T, T> {
    final int count;

    public FlowableTakeLast(Flowable<T> source, int count) {
        super(source);
        this.count = count;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new TakeLastSubscriber<T>(s, count));
    }

    static final class TakeLastSubscriber<T> extends ArrayDeque<T> implements FlowableSubscriber<T>, Subscription {

        private static final long serialVersionUID = 7240042530241604978L;
        final Subscriber<? super T> actual;
        final int count;

        Subscription s;
        volatile boolean done;
        volatile boolean cancelled;

        final AtomicLong requested = new AtomicLong();

        final AtomicInteger wip = new AtomicInteger();

        TakeLastSubscriber(Subscriber<? super T> actual, int count) {
            this.actual = actual;
            this.count = count;
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
            if (count == size()) {
                poll();
            }
            offer(t);
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
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
                drain();
            }
        }

        @Override
        public void cancel() {
            cancelled = true;
            s.cancel();
        }

        void drain() {
            if (wip.getAndIncrement() == 0) {
                Subscriber<? super T> a = actual;
                long r = requested.get();
                do {
                    if (cancelled) {
                        return;
                    }
                    if (done) {
                        long e = 0L;

                        while (e != r) {
                            if (cancelled) {
                                return;
                            }
                            T v = poll();
                            if (v == null) {
                                a.onComplete();
                                return;
                            }
                            a.onNext(v);
                            e++;
                        }
                        if (e != 0L && r != Long.MAX_VALUE) {
                            r = requested.addAndGet(-e);
                        }
                    }
                } while (wip.decrementAndGet() != 0);
            }
        }
    }
}
