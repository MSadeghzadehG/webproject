

package io.reactivex.internal.operators.single;

import io.reactivex.Single;
import io.reactivex.SingleObserver;
import io.reactivex.SingleSource;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Action;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class SingleDoAfterTerminate<T> extends Single<T> {

    final SingleSource<T> source;

    final Action onAfterTerminate;

    public SingleDoAfterTerminate(SingleSource<T> source, Action onAfterTerminate) {
        this.source = source;
        this.onAfterTerminate = onAfterTerminate;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> s) {
        source.subscribe(new DoAfterTerminateObserver<T>(s, onAfterTerminate));
    }

    static final class DoAfterTerminateObserver<T> implements SingleObserver<T>, Disposable {

        final SingleObserver<? super T> actual;

        final Action onAfterTerminate;

        Disposable d;

        DoAfterTerminateObserver(SingleObserver<? super T> actual, Action onAfterTerminate) {
            this.actual = actual;
            this.onAfterTerminate = onAfterTerminate;
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

            onAfterTerminate();
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);

            onAfterTerminate();
        }

        @Override
        public void dispose() {
            d.dispose();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        private void onAfterTerminate() {
            try {
                onAfterTerminate.run();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                RxJavaPlugins.onError(ex);
            }
        }
    }
}
