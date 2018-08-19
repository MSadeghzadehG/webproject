

package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.*;

import org.reactivestreams.Subscription;

import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class AsyncSubscription extends AtomicLong implements Subscription, Disposable {

    private static final long serialVersionUID = 7028635084060361255L;

    final AtomicReference<Subscription> actual;

    final AtomicReference<Disposable> resource;

    public AsyncSubscription() {
        resource = new AtomicReference<Disposable>();
        actual = new AtomicReference<Subscription>();
    }

    public AsyncSubscription(Disposable resource) {
        this();
        this.resource.lazySet(resource);
    }

    @Override
    public void request(long n) {
        SubscriptionHelper.deferredRequest(actual, this, n);
    }

    @Override
    public void cancel() {
        dispose();
    }

    @Override
    public void dispose() {
        SubscriptionHelper.cancel(actual);
        DisposableHelper.dispose(resource);
    }

    @Override
    public boolean isDisposed() {
        return actual.get() == SubscriptionHelper.CANCELLED;
    }

    
    public boolean setResource(Disposable r) {
        return DisposableHelper.set(resource, r);
    }

    
    public boolean replaceResource(Disposable r) {
        return DisposableHelper.replace(resource, r);
    }

    
    public void setSubscription(Subscription s) {
        SubscriptionHelper.deferredSetOnce(actual, this, s);
    }
}
