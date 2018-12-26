

package io.reactivex.internal.subscribers;

import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.Subscription;

import io.reactivex.FlowableSubscriber;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.*;
import io.reactivex.functions.*;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class ForEachWhileSubscriber<T>
extends AtomicReference<Subscription>
implements FlowableSubscriber<T>, Disposable {


    private static final long serialVersionUID = -4403180040475402120L;

    final Predicate<? super T> onNext;

    final Consumer<? super Throwable> onError;

    final Action onComplete;

    boolean done;

    public ForEachWhileSubscriber(Predicate<? super T> onNext,
            Consumer<? super Throwable> onError, Action onComplete) {
        this.onNext = onNext;
        this.onError = onError;
        this.onComplete = onComplete;
    }

    @Override
    public void onSubscribe(Subscription s) {
        SubscriptionHelper.setOnce(this, s, Long.MAX_VALUE);
    }

    @Override
    public void onNext(T t) {
        if (done) {
            return;
        }

        boolean b;
        try {
            b = onNext.test(t);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            dispose();
            onError(ex);
            return;
        }

        if (!b) {
            dispose();
            onComplete();
        }
    }

    @Override
    public void onError(Throwable t) {
        if (done) {
            RxJavaPlugins.onError(t);
            return;
        }
        done = true;
        try {
            onError.accept(t);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            RxJavaPlugins.onError(new CompositeException(t, ex));
        }
    }

    @Override
    public void onComplete() {
        if (done) {
            return;
        }
        done = true;
        try {
            onComplete.run();
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            RxJavaPlugins.onError(ex);
        }
    }

    @Override
    public void dispose() {
        SubscriptionHelper.cancel(this);
    }

    @Override
    public boolean isDisposed() {
        return SubscriptionHelper.isCancelled(this.get());
    }
}
