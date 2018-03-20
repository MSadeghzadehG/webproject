

package io.reactivex.internal.operators.completable;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;

public final class CompletableToSingle<T> extends Single<T> {
    final CompletableSource source;

    final Callable<? extends T> completionValueSupplier;

    final T completionValue;

    public CompletableToSingle(CompletableSource source,
            Callable<? extends T> completionValueSupplier, T completionValue) {
        this.source = source;
        this.completionValue = completionValue;
        this.completionValueSupplier = completionValueSupplier;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super T> s) {
        source.subscribe(new ToSingle(s));
    }

    final class ToSingle implements CompletableObserver {

        private final SingleObserver<? super T> observer;

        ToSingle(SingleObserver<? super T> observer) {
            this.observer = observer;
        }

        @Override
        public void onComplete() {
            T v;

            if (completionValueSupplier != null) {
                try {
                    v = completionValueSupplier.call();
                } catch (Throwable e) {
                    Exceptions.throwIfFatal(e);
                    observer.onError(e);
                    return;
                }
            } else {
                v = completionValue;
            }

            if (v == null) {
                observer.onError(new NullPointerException("The value supplied is null"));
            } else {
                observer.onSuccess(v);
            }
        }

        @Override
        public void onError(Throwable e) {
            observer.onError(e);
        }

        @Override
        public void onSubscribe(Disposable d) {
            observer.onSubscribe(d);
        }

    }
}
