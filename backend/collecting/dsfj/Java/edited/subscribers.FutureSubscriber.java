

package io.reactivex.internal.subscribers;

import java.util.NoSuchElementException;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.Subscription;

import io.reactivex.FlowableSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.BlockingHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class FutureSubscriber<T> extends CountDownLatch
implements FlowableSubscriber<T>, Future<T>, Subscription {

    T value;
    Throwable error;

    final AtomicReference<Subscription> s;

    public FutureSubscriber() {
        super(1);
        this.s = new AtomicReference<Subscription>();
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        for (;;) {
            Subscription a = s.get();
            if (a == this || a == SubscriptionHelper.CANCELLED) {
                return false;
            }

            if (s.compareAndSet(a, SubscriptionHelper.CANCELLED)) {
                if (a != null) {
                    a.cancel();
                }
                countDown();
                return true;
            }
        }
    }

    @Override
    public boolean isCancelled() {
        return SubscriptionHelper.isCancelled(s.get());
    }

    @Override
    public boolean isDone() {
        return getCount() == 0;
    }

    @Override
    public T get() throws InterruptedException, ExecutionException {
        if (getCount() != 0) {
            BlockingHelper.verifyNonBlocking();
            await();
        }

        if (isCancelled()) {
            throw new CancellationException();
        }
        Throwable ex = error;
        if (ex != null) {
            throw new ExecutionException(ex);
        }
        return value;
    }

    @Override
    public T get(long timeout, TimeUnit unit) throws InterruptedException, ExecutionException, TimeoutException {
        if (getCount() != 0) {
            BlockingHelper.verifyNonBlocking();
            if (!await(timeout, unit)) {
                throw new TimeoutException();
            }
        }

        if (isCancelled()) {
            throw new CancellationException();
        }

        Throwable ex = error;
        if (ex != null) {
            throw new ExecutionException(ex);
        }
        return value;
    }

    @Override
    public void onSubscribe(Subscription s) {
        SubscriptionHelper.setOnce(this.s, s, Long.MAX_VALUE);
    }

    @Override
    public void onNext(T t) {
        if (value != null) {
            s.get().cancel();
            onError(new IndexOutOfBoundsException("More than one element received"));
            return;
        }
        value = t;
    }

    @Override
    public void onError(Throwable t) {
        for (;;) {
            Subscription a = s.get();
            if (a == this || a == SubscriptionHelper.CANCELLED) {
                RxJavaPlugins.onError(t);
                return;
            }
            error = t;
            if (s.compareAndSet(a, this)) {
                countDown();
                return;
            }
        }
    }

    @Override
    public void onComplete() {
        if (value == null) {
            onError(new NoSuchElementException("The source is empty"));
            return;
        }
        for (;;) {
            Subscription a = s.get();
            if (a == this || a == SubscriptionHelper.CANCELLED) {
                return;
            }
            if (s.compareAndSet(a, this)) {
                countDown();
                return;
            }
        }
    }

    @Override
    public void cancel() {
            }

    @Override
    public void request(long n) {
            }
}
