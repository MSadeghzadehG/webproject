

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicInteger;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.internal.subscriptions.SubscriptionArbiter;

public final class FlowableRepeat<T> extends AbstractFlowableWithUpstream<T, T> {
    final long count;
    public FlowableRepeat(Flowable<T> source, long count) {
        super(source);
        this.count = count;
    }

    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        SubscriptionArbiter sa = new SubscriptionArbiter();
        s.onSubscribe(sa);

        RepeatSubscriber<T> rs = new RepeatSubscriber<T>(s, count != Long.MAX_VALUE ? count - 1 : Long.MAX_VALUE, sa, source);
        rs.subscribeNext();
    }

    static final class RepeatSubscriber<T> extends AtomicInteger implements FlowableSubscriber<T> {

        private static final long serialVersionUID = -7098360935104053232L;

        final Subscriber<? super T> actual;
        final SubscriptionArbiter sa;
        final Publisher<? extends T> source;
        long remaining;

        long produced;

        RepeatSubscriber(Subscriber<? super T> actual, long count, SubscriptionArbiter sa, Publisher<? extends T> source) {
            this.actual = actual;
            this.sa = sa;
            this.source = source;
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
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            long r = remaining;
            if (r != Long.MAX_VALUE) {
                remaining = r - 1;
            }
            if (r != 0L) {
                subscribeNext();
            } else {
                actual.onComplete();
            }
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
