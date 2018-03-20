

package io.reactivex.internal.operators.maybe;

import java.util.NoSuchElementException;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.HasUpstreamMaybeSource;


public final class MaybeToSingle<T> extends Single<T> implements HasUpstreamMaybeSource<T> {

    final MaybeSource<T> source;
    final T defaultValue;

    public MaybeToSingle(MaybeSource<T> source, T defaultValue) {
        this.source = source;
        this.defaultValue = defaultValue;
    }

    @Override
    public MaybeSource<T> source() {
        return source;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> observer) {
        source.subscribe(new ToSingleMaybeSubscriber<T>(observer, defaultValue));
    }

    static final class ToSingleMaybeSubscriber<T> implements MaybeObserver<T>, Disposable {
        final SingleObserver<? super T> actual;
        final T defaultValue;

        Disposable d;

        ToSingleMaybeSubscriber(SingleObserver<? super T> actual, T defaultValue) {
            this.actual = actual;
            this.defaultValue = defaultValue;
        }

        @Override
        public void dispose() {
            d.dispose();
            d = DisposableHelper.DISPOSED;
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            d = DisposableHelper.DISPOSED;
            actual.onSuccess(value);
        }

        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            if (defaultValue != null) {
                actual.onSuccess(defaultValue);
            } else {
                actual.onError(new NoSuchElementException("The MaybeSource is empty"));
            }
        }
    }
}