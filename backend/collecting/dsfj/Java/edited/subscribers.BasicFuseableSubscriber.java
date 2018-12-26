

package io.reactivex.internal.subscribers;

import org.reactivestreams.*;

import io.reactivex.FlowableSubscriber;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.fuseable.QueueSubscription;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;


public abstract class BasicFuseableSubscriber<T, R> implements FlowableSubscriber<T>, QueueSubscription<R> {

    
    protected final Subscriber<? super R> actual;

    
    protected Subscription s;

    
    protected QueueSubscription<T> qs;

    
    protected boolean done;

    
    protected int sourceMode;

    
    public BasicFuseableSubscriber(Subscriber<? super R> actual) {
        this.actual = actual;
    }

        @SuppressWarnings("unchecked")
    @Override
    public final void onSubscribe(Subscription s) {
        if (SubscriptionHelper.validate(this.s, s)) {

            this.s = s;
            if (s instanceof QueueSubscription) {
                this.qs = (QueueSubscription<T>)s;
            }

            if (beforeDownstream()) {

                actual.onSubscribe(this);

                afterDownstream();
            }

        }
    }

    
    protected boolean beforeDownstream() {
        return true;
    }

    
    protected void afterDownstream() {
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

    
    protected final void fail(Throwable t) {
        Exceptions.throwIfFatal(t);
        s.cancel();
        onError(t);
    }

    @Override
    public void onComplete() {
        if (done) {
            return;
        }
        done = true;
        actual.onComplete();
    }

    
    protected final int transitiveBoundaryFusion(int mode) {
        QueueSubscription<T> qs = this.qs;
        if (qs != null) {
            if ((mode & BOUNDARY) == 0) {
                int m = qs.requestFusion(mode);
                if (m != NONE) {
                    sourceMode = m;
                }
                return m;
            }
        }
        return NONE;
    }

            
    @Override
    public void request(long n) {
        s.request(n);
    }

    @Override
    public void cancel() {
        s.cancel();
    }

    @Override
    public boolean isEmpty() {
        return qs.isEmpty();
    }

    @Override
    public void clear() {
        qs.clear();
    }

            
    @Override
    public final boolean offer(R e) {
        throw new UnsupportedOperationException("Should not be called!");
    }

    @Override
    public final boolean offer(R v1, R v2) {
        throw new UnsupportedOperationException("Should not be called!");
    }
}
