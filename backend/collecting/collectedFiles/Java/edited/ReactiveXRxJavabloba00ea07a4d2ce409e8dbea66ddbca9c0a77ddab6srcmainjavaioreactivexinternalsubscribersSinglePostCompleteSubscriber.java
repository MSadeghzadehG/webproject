

package io.reactivex.internal.subscribers;

import java.util.concurrent.atomic.AtomicLong;

import org.reactivestreams.*;

import io.reactivex.FlowableSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.BackpressureHelper;


public abstract class SinglePostCompleteSubscriber<T, R> extends AtomicLong implements FlowableSubscriber<T>, Subscription {
    private static final long serialVersionUID = 7917814472626990048L;

    
    protected final Subscriber<? super R> actual;

    
    protected Subscription s;

    
    protected R value;

    
    protected long produced;

    
    static final long COMPLETE_MASK = Long.MIN_VALUE;
    
    static final long REQUEST_MASK = Long.MAX_VALUE;

    public SinglePostCompleteSubscriber(Subscriber<? super R> actual) {
        this.actual = actual;
    }

    @Override
    public void onSubscribe(Subscription s) {
        if (SubscriptionHelper.validate(this.s, s)) {
            this.s = s;
            actual.onSubscribe(this);
        }
    }

    
    protected final void complete(R n) {
        long p = produced;
        if (p != 0) {
            BackpressureHelper.produced(this, p);
        }

        for (;;) {
            long r = get();
            if ((r & COMPLETE_MASK) != 0) {
                onDrop(n);
                return;
            }
            if ((r & REQUEST_MASK) != 0) {
                lazySet(COMPLETE_MASK + 1);
                actual.onNext(n);
                actual.onComplete();
                return;
            }
            value = n;
            if (compareAndSet(0, COMPLETE_MASK)) {
                return;
            }
            value = null;
        }
    }

    
    protected void onDrop(R n) {
            }

    @Override
    public final void request(long n) {
        if (SubscriptionHelper.validate(n)) {
            for (;;) {
                long r = get();
                if ((r & COMPLETE_MASK) != 0) {
                    if (compareAndSet(COMPLETE_MASK, COMPLETE_MASK + 1)) {
                        actual.onNext(value);
                        actual.onComplete();
                    }
                    break;
                }
                long u = BackpressureHelper.addCap(r, n);
                if (compareAndSet(r, u)) {
                    s.request(n);
                    break;
                }
            }
        }
    }

    @Override
    public void cancel() {
        s.cancel();
    }
}
