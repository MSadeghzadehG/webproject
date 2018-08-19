

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class MaybeDelaySubscriptionOtherPublisher<T, U> extends AbstractMaybeWithUpstream<T, T> {

    final Publisher<U> other;

    public MaybeDelaySubscriptionOtherPublisher(MaybeSource<T> source, Publisher<U> other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        other.subscribe(new OtherSubscriber<T>(observer, source));
    }

    static final class OtherSubscriber<T> implements FlowableSubscriber<Object>, Disposable {
        final DelayMaybeObserver<T> main;

        MaybeSource<T> source;

        Subscription s;

        OtherSubscriber(MaybeObserver<? super T> actual, MaybeSource<T> source) {
            this.main = new DelayMaybeObserver<T>(actual);
            this.source = source;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;

                main.actual.onSubscribe(this);

                s.request(Long.MAX_VALUE);
            }
        }

        @Override
        public void onNext(Object t) {
            if (s != SubscriptionHelper.CANCELLED) {
                s.cancel();
                s = SubscriptionHelper.CANCELLED;

                subscribeNext();
            }
        }

        @Override
        public void onError(Throwable t) {
            if (s != SubscriptionHelper.CANCELLED) {
                s = SubscriptionHelper.CANCELLED;

                main.actual.onError(t);
            } else {
                RxJavaPlugins.onError(t);
            }
        }

        @Override
        public void onComplete() {
            if (s != SubscriptionHelper.CANCELLED) {
                s = SubscriptionHelper.CANCELLED;

                subscribeNext();
            }
        }

        void subscribeNext() {
            MaybeSource<T> src = source;
            source = null;

            src.subscribe(main);
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(main.get());
        }

        @Override
        public void dispose() {
            s.cancel();
            s = SubscriptionHelper.CANCELLED;
            DisposableHelper.dispose(main);
        }
    }

    static final class DelayMaybeObserver<T> extends AtomicReference<Disposable>
    implements MaybeObserver<T> {

        private static final long serialVersionUID = 706635022205076709L;

        final MaybeObserver<? super T> actual;

        DelayMaybeObserver(MaybeObserver<? super T> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(this, d);
        }

        @Override
        public void onSuccess(T value) {
            actual.onSuccess(value);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }
    }
}
