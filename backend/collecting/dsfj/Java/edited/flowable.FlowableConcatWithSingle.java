

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.subscribers.SinglePostCompleteSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class FlowableConcatWithSingle<T> extends AbstractFlowableWithUpstream<T, T> {

    final SingleSource<? extends T> other;

    public FlowableConcatWithSingle(Flowable<T> source, SingleSource<? extends T> other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        source.subscribe(new ConcatWithSubscriber<T>(s, other));
    }

    static final class ConcatWithSubscriber<T>
    extends SinglePostCompleteSubscriber<T, T>
    implements SingleObserver<T> {

        private static final long serialVersionUID = -7346385463600070225L;

        final AtomicReference<Disposable> otherDisposable;

        SingleSource<? extends T> other;

        ConcatWithSubscriber(Subscriber<? super T> actual, SingleSource<? extends T> other) {
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
            s = SubscriptionHelper.CANCELLED;
            SingleSource<? extends T> ss = other;
            other = null;
            ss.subscribe(this);
        }

        @Override
        public void cancel() {
            super.cancel();
            DisposableHelper.dispose(otherDisposable);
        }
    }
}
