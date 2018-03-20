
package io.reactivex.internal.subscribers;

import java.util.concurrent.CountDownLatch;

import org.reactivestreams.Subscription;

import io.reactivex.FlowableSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.*;

public abstract class BlockingBaseSubscriber<T> extends CountDownLatch
implements FlowableSubscriber<T> {

    T value;
    Throwable error;

    Subscription s;

    volatile boolean cancelled;

    public BlockingBaseSubscriber() {
        super(1);
    }

    @Override
    public final void onSubscribe(Subscription s) {
        if (SubscriptionHelper.validate(this.s, s)) {
            this.s = s;
            if (!cancelled) {
                s.request(Long.MAX_VALUE);
                if (cancelled) {
                    this.s = SubscriptionHelper.CANCELLED;
                    s.cancel();
                }
            }
        }
    }

    @Override
    public final void onComplete() {
        countDown();
    }

    
    public final T blockingGet() {
        if (getCount() != 0) {
            try {
                BlockingHelper.verifyNonBlocking();
                await();
            } catch (InterruptedException ex) {
                Subscription s = this.s;
                this.s = SubscriptionHelper.CANCELLED;
                if (s != null) {
                    s.cancel();
                }
                throw ExceptionHelper.wrapOrThrow(ex);
            }
        }

        Throwable e = error;
        if (e != null) {
            throw ExceptionHelper.wrapOrThrow(e);
        }
        return value;
    }
}
