

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BiFunction;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class ObservableReduceSeedSingle<T, R> extends Single<R> {

    final ObservableSource<T> source;

    final R seed;

    final BiFunction<R, ? super T, R> reducer;

    public ObservableReduceSeedSingle(ObservableSource<T> source, R seed, BiFunction<R, ? super T, R> reducer) {
        this.source = source;
        this.seed = seed;
        this.reducer = reducer;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super R> observer) {
        source.subscribe(new ReduceSeedObserver<T, R>(observer, reducer, seed));
    }

    static final class ReduceSeedObserver<T, R> implements Observer<T>, Disposable {

        final SingleObserver<? super R> actual;

        final BiFunction<R, ? super T, R> reducer;

        R value;

        Disposable d;

        ReduceSeedObserver(SingleObserver<? super R> actual, BiFunction<R, ? super T, R> reducer, R value) {
            this.actual = actual;
            this.value = value;
            this.reducer = reducer;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T value) {
            R v = this.value;
            if (v != null) {
                try {
                    this.value = ObjectHelper.requireNonNull(reducer.apply(v, value), "The reducer returned a null value");
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    d.dispose();
                    onError(ex);
                }
            }
        }

        @Override
        public void onError(Throwable e) {
            R v = value;
            if (v != null) {
                value = null;
                actual.onError(e);
            } else {
                RxJavaPlugins.onError(e);
            }
        }

        @Override
        public void onComplete() {
            R v = value;
            if (v != null) {
                value = null;
                actual.onSuccess(v);
            }
        }

        @Override
        public void dispose() {
            d.dispose();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }
    }
}
