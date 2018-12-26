

package io.reactivex.internal.operators.observable;

import java.util.concurrent.*;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.observers.DeferredScalarDisposable;

public final class ObservableFromFuture<T> extends Observable<T> {
    final Future<? extends T> future;
    final long timeout;
    final TimeUnit unit;

    public ObservableFromFuture(Future<? extends T> future, long timeout, TimeUnit unit) {
        this.future = future;
        this.timeout = timeout;
        this.unit = unit;
    }

    @Override
    public void subscribeActual(Observer<? super T> s) {
        DeferredScalarDisposable<T> d = new DeferredScalarDisposable<T>(s);
        s.onSubscribe(d);
        if (!d.isDisposed()) {
            T v;
            try {
                v = ObjectHelper.requireNonNull(unit != null ? future.get(timeout, unit) : future.get(), "Future returned null");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                if (!d.isDisposed()) {
                    s.onError(ex);
                }
                return;
            }
            d.complete(v);
        }
    }
}
