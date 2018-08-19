

package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.AtomicReferenceArray;

import org.reactivestreams.Subscription;

import io.reactivex.disposables.Disposable;


public final class ArrayCompositeSubscription extends AtomicReferenceArray<Subscription> implements Disposable {

    private static final long serialVersionUID = 2746389416410565408L;

    public ArrayCompositeSubscription(int capacity) {
        super(capacity);
    }

    
    public boolean setResource(int index, Subscription resource) {
        for (;;) {
            Subscription o = get(index);
            if (o == SubscriptionHelper.CANCELLED) {
                if (resource != null) {
                    resource.cancel();
                }
                return false;
            }
            if (compareAndSet(index, o, resource)) {
                if (o != null) {
                    o.cancel();
                }
                return true;
            }
        }
    }

    
    public Subscription replaceResource(int index, Subscription resource) {
        for (;;) {
            Subscription o = get(index);
            if (o == SubscriptionHelper.CANCELLED) {
                if (resource != null) {
                    resource.cancel();
                }
                return null;
            }
            if (compareAndSet(index, o, resource)) {
                return o;
            }
        }
    }

    @Override
    public void dispose() {
        if (get(0) != SubscriptionHelper.CANCELLED) {
            int s = length();
            for (int i = 0; i < s; i++) {
                Subscription o = get(i);
                if (o != SubscriptionHelper.CANCELLED) {
                    o = getAndSet(i, SubscriptionHelper.CANCELLED);
                    if (o != SubscriptionHelper.CANCELLED && o != null) {
                        o.cancel();
                    }
                }
            }
        }
    }

    @Override
    public boolean isDisposed() {
        return get(0) == SubscriptionHelper.CANCELLED;
    }
}
