

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.CompositeException;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class MaybeDelayOtherPublisher<T, U> extends AbstractMaybeWithUpstream<T, T> {

    final Publisher<U> other;

    public MaybeDelayOtherPublisher(MaybeSource<T> source, Publisher<U> other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(new DelayMaybeObserver<T, U>(observer, other));
    }

    static final class DelayMaybeObserver<T, U>
    implements MaybeObserver<T>, Disposable {
        final OtherSubscriber<T> other;

        final Publisher<U> otherSource;

        Disposable d;

        DelayMaybeObserver(MaybeObserver<? super T> actual, Publisher<U> otherSource) {
            this.other = new OtherSubscriber<T>(actual);
            this.otherSource = otherSource;
        }

        @Override
        public void dispose() {
            d.dispose();
            d = DisposableHelper.DISPOSED;
            SubscriptionHelper.cancel(other);
        }

        @Override
        public boolean isDisposed() {
            return SubscriptionHelper.isCancelled(other.get());
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                other.actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            d = DisposableHelper.DISPOSED;
            other.value = value;
            subscribeNext();
        }

        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            other.error = e;
            subscribeNext();
        }

        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            subscribeNext();
        }

        void subscribeNext() {
            otherSource.subscribe(other);
        }
    }

    static final class OtherSubscriber<T> extends
    AtomicReference<Subscription>
    implements FlowableSubscriber<Object> {

        private static final long serialVersionUID = -1215060610805418006L;

        final MaybeObserver<? super T> actual;

        T value;

        Throwable error;

        OtherSubscriber(MaybeObserver<? super T> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Subscription s) {
            SubscriptionHelper.setOnce(this, s, Long.MAX_VALUE);
        }

        @Override
        public void onNext(Object t) {
            Subscription s = get();
            if (s != SubscriptionHelper.CANCELLED) {
                lazySet(SubscriptionHelper.CANCELLED);
                s.cancel();
                onComplete();
            }
        }

        @Override
        public void onError(Throwable t) {
            Throwable e = error;
            if (e == null) {
                actual.onError(t);
            } else {
                actual.onError(new CompositeException(e, t));
            }
        }

        @Override
        public void onComplete() {
            Throwable e = error;
            if (e != null) {
                actual.onError(e);
            } else {
                T v = value;
                if (v != null) {
                    actual.onSuccess(v);
                } else {
                    actual.onComplete();
                }
            }
        }
    }
}
