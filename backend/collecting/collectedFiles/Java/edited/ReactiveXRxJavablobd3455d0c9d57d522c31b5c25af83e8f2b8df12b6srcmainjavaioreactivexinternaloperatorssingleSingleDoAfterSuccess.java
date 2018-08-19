

package io.reactivex.internal.operators.single;

import io.reactivex.*;
import io.reactivex.annotations.Experimental;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;


@Experimental
public final class SingleDoAfterSuccess<T> extends Single<T> {

    final SingleSource<T> source;

    final Consumer<? super T> onAfterSuccess;

    public SingleDoAfterSuccess(SingleSource<T> source, Consumer<? super T> onAfterSuccess) {
        this.source = source;
        this.onAfterSuccess = onAfterSuccess;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> s) {
        source.subscribe(new DoAfterObserver<T>(s, onAfterSuccess));
    }

    static final class DoAfterObserver<T> implements SingleObserver<T>, Disposable {

        final SingleObserver<? super T> actual;

        final Consumer<? super T> onAfterSuccess;

        Disposable d;

        DoAfterObserver(SingleObserver<? super T> actual, Consumer<? super T> onAfterSuccess) {
            this.actual = actual;
            this.onAfterSuccess = onAfterSuccess;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T t) {
            actual.onSuccess(t);

            try {
                onAfterSuccess.accept(t);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                             RxJavaPlugins.onError(ex);
            }
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
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
