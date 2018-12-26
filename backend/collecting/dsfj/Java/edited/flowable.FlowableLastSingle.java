

package io.reactivex.internal.operators.flowable;

import java.util.NoSuchElementException;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class FlowableLastSingle<T> extends Single<T> {

    final Publisher<T> source;

    final T defaultItem;

    public FlowableLastSingle(Publisher<T> source, T defaultItem) {
        this.source = source;
        this.defaultItem = defaultItem;
    }

    
    @Override
    protected void subscribeActual(SingleObserver<? super T> observer) {
        source.subscribe(new LastSubscriber<T>(observer, defaultItem));
    }

    static final class LastSubscriber<T> implements FlowableSubscriber<T>, Disposable {

        final SingleObserver<? super T> actual;

        final T defaultItem;

        Subscription s;

        T item;

        LastSubscriber(SingleObserver<? super T> actual, T defaultItem) {
            this.actual = actual;
            this.defaultItem = defaultItem;
        }

        @Override
        public void dispose() {
            s.cancel();
            s = SubscriptionHelper.CANCELLED;
        }

        @Override
        public boolean isDisposed() {
            return s == SubscriptionHelper.CANCELLED;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;

                actual.onSubscribe(this);

                s.request(Long.MAX_VALUE);
            }
        }

        @Override
        public void onNext(T t) {
            item = t;
        }

        @Override
        public void onError(Throwable t) {
            s = SubscriptionHelper.CANCELLED;
            item = null;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            s = SubscriptionHelper.CANCELLED;
            T v = item;
            if (v != null) {
                item = null;
                actual.onSuccess(v);
            } else {
                v = defaultItem;

                if (v != null) {
                    actual.onSuccess(v);
                } else {
                    actual.onError(new NoSuchElementException());
                }
            }
        }
    }
}
