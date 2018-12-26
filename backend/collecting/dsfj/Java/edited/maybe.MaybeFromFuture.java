

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.*;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.Exceptions;


public final class MaybeFromFuture<T> extends Maybe<T> {

    final Future<? extends T> future;

    final long timeout;

    final TimeUnit unit;

    public MaybeFromFuture(Future<? extends T> future, long timeout, TimeUnit unit) {
        this.future = future;
        this.timeout = timeout;
        this.unit = unit;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        Disposable d = Disposables.empty();
        observer.onSubscribe(d);
        if (!d.isDisposed()) {
            T v;
            try {
                if (timeout <= 0L) {
                    v = future.get();
                } else {
                    v = future.get(timeout, unit);
                }
            } catch (Throwable ex) {
                if (ex instanceof ExecutionException) {
                    ex = ex.getCause();
                }
                Exceptions.throwIfFatal(ex);
                if (!d.isDisposed()) {
                    observer.onError(ex);
                }
                return;
            }
            if (!d.isDisposed()) {
                if (v == null) {
                    observer.onComplete();
                } else {
                    observer.onSuccess(v);
                }
            }
        }
    }
}
