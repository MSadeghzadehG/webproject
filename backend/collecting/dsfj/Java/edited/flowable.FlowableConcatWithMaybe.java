

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.subscribers.SinglePostCompleteSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class FlowableConcatWithMaybe<T> extends AbstractFlowableWithUpstream<T, T> {

    final MaybeSource<? extends T> other;

    public FlowableConcatWithMaybe(Flowable<T> source, MaybeSource<? extends T> other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new ConcatWithSubscriber<T>(s, other));
    }

    static final class ConcatWithSubscriber<T>
    extends SinglePostCompleteSubscriber<T, T>
    implements MaybeObserver<T> {

        private static final long serialVersionUID = -7346385463600070225L;

        final AtomicReference<Disposable> otherDisposable;

        MaybeSource<? extends T> other;

        boolean inMaybe;

        ConcatWithSubscriber(Subscriber<? super T> actual, MaybeSource<? extends T> other) {
            super(actual);
            this.other = other;
            this.otherDisposable = new AtomicReference<Disposable>();
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(otherDisposable, d);
        }

        @Override
        public void onNext(T t) {
            produced++;
            actual.onNext(t);
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
        }

        @Override
        public void onSuccess(T t) {
            complete(t);
        }

        @Override
        public void onComplete() {
            if (inMaybe) {
                actual.onComplete();
            } else {
                inMaybe = true;
                s = SubscriptionHelper.CANCELLED;
                MaybeSource<? extends T> ms = other;
                other = null;
                ms.subscribe(this);
            }
        }

        @Override
        public void cancel() {
            super.cancel();
            DisposableHelper.dispose(otherDisposable);
        }
    }
}
