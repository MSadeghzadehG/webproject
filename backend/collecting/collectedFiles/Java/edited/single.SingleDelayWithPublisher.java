

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.observers.ResumeSingleObserver;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class SingleDelayWithPublisher<T, U> extends Single<T> {

    final SingleSource<T> source;

    final Publisher<U> other;

    public SingleDelayWithPublisher(SingleSource<T> source, Publisher<U> other) {
        this.source = source;
        this.other = other;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> subscriber) {
        other.subscribe(new OtherSubscriber<T, U>(subscriber, source));
    }

    static final class OtherSubscriber<T, U>
    extends AtomicReference<Disposable>
    implements FlowableSubscriber<U>, Disposable {


        private static final long serialVersionUID = -8565274649390031272L;

        final SingleObserver<? super T> actual;

        final SingleSource<T> source;

        boolean done;

        Subscription s;

        OtherSubscriber(SingleObserver<? super T> actual, SingleSource<T> source) {
            this.actual = actual;
            this.source = source;
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
        public void onNext(U value) {
            s.cancel();
            onComplete();
        }

        @Override
        public void onError(Throwable e) {
            if (done) {
                RxJavaPlugins.onError(e);
                return;
            }
            done = true;
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            source.subscribe(new ResumeSingleObserver<T>(this, actual));
        }

        @Override
        public void dispose() {
            s.cancel();
            DisposableHelper.dispose(this);
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }
    }
}
