

package io.reactivex.internal.observers;

import org.reactivestreams.*;

import io.reactivex.CompletableObserver;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;

public final class SubscriberCompletableObserver<T> implements CompletableObserver, Subscription {
    final Subscriber<? super T> subscriber;

    Disposable d;

    public SubscriberCompletableObserver(Subscriber<? super T> observer) {
        this.subscriber = observer;
    }

    @Override
    public void onComplete() {
        subscriber.onComplete();
    }

    @Override
    public void onError(Throwable e) {
        subscriber.onError(e);
    }

    @Override
    public void onSubscribe(Disposable d) {
        if (DisposableHelper.validate(this.d, d)) {
            this.d = d;

            subscriber.onSubscribe(this);
        }
    }

    @Override
    public void request(long n) {
            }

    @Override
    public void cancel() {
        d.dispose();
    }
}
