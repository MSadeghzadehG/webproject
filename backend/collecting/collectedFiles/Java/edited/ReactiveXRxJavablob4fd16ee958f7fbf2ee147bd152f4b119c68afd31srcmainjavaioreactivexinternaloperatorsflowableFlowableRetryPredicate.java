

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicInteger;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.exceptions.*;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.subscriptions.SubscriptionArbiter;

public final class FlowableRetryPredicate<T> extends AbstractFlowableWithUpstream<T, T> {
    final Predicate<? super Throwable> predicate;
    final long count;
    public FlowableRetryPredicate(Flowable<T> source,
            long count,
            Predicate<? super Throwable> predicate) {
        super(source);
        this.predicate = predicate;
        this.count = count;
    }

    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        SubscriptionArbiter sa = new SubscriptionArbiter();
        s.onSubscribe(sa);

        RetrySubscriber<T> rs = new RetrySubscriber<T>(s, count, predicate, sa, source);
        rs.subscribeNext();
    }

    static final class RetrySubscriber<T> extends AtomicInteger implements FlowableSubscriber<T> {

        private static final long serialVersionUID = -7098360935104053232L;

        final Subscriber<? super T> actual;
        final SubscriptionArbiter sa;
        final Publisher<? extends T> source;
        final Predicate<? super Throwable> predicate;
        long remaining;

        long produced;

        RetrySubscriber(Subscriber<? super T> actual, long count,
                Predicate<? super Throwable> predicate, SubscriptionArbiter sa, Publisher<? extends T> source) {
            this.actual = actual;
            this.sa = sa;
            this.source = source;
            this.predicate = predicate;
            this.remaining = count;
        }

        @Override
        public void onSubscribe(Subscription s) {
            sa.setSubscription(s);
        }

        @Override
        public void onNext(T t) {
            produced++;
            actual.onNext(t);
        }
        @Override
        public void onError(Throwable t) {
            long r = remaining;
            if (r != Long.MAX_VALUE) {
                remaining = r - 1;
            }
            if (r == 0) {
                actual.onError(t);
            } else {
                boolean b;
                try {
                    b = predicate.test(t);
                } catch (Throwable e) {
                    Exceptions.throwIfFatal(e);
                    actual.onError(new CompositeException(t, e));
                    return;
                }
                if (!b) {
                    actual.onError(t);
                    return;
                }
                subscribeNext();
            }
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }

        
        void subscribeNext() {
            if (getAndIncrement() == 0) {
                int missed = 1;
                for (;;) {
                    if (sa.isCancelled()) {
                        return;
                    }

                    long p = produced;
                    if (p != 0L) {
                        produced = 0L;
                        sa.produced(p);
                    }

                    source.subscribe(this);

                    missed = addAndGet(-missed);
                    if (missed == 0) {
                        break;
                    }
                }
            }
        }
    }
}
