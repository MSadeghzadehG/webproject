

package io.reactivex.internal.subscribers;

import java.util.Queue;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.Subscription;

import io.reactivex.FlowableSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.NotificationLite;

public final class BlockingSubscriber<T> extends AtomicReference<Subscription> implements FlowableSubscriber<T>, Subscription {

    private static final long serialVersionUID = -4875965440900746268L;

    public static final Object TERMINATED = new Object();

    final Queue<Object> queue;

    public BlockingSubscriber(Queue<Object> queue) {
        this.queue = queue;
    }

    @Override
    public void onSubscribe(Subscription s) {
        if (SubscriptionHelper.setOnce(this, s)) {
            queue.offer(NotificationLite.subscription(this));
        }
    }

    @Override
    public void onNext(T t) {
        queue.offer(NotificationLite.next(t));
    }

    @Override
    public void onError(Throwable t) {
        queue.offer(NotificationLite.error(t));
    }

    @Override
    public void onComplete() {
        queue.offer(NotificationLite.complete());
    }

    @Override
    public void request(long n) {
        get().request(n);
    }

    @Override
    public void cancel() {
        if (SubscriptionHelper.cancel(this)) {
            queue.offer(TERMINATED);
        }
    }

    public boolean isCancelled() {
        return get() == SubscriptionHelper.CANCELLED;
    }
}
