

package io.reactivex.internal.operators.observable;

import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;

public final class ObservableError<T> extends Observable<T> {
    final Callable<? extends Throwable> errorSupplier;
    public ObservableError(Callable<? extends Throwable> errorSupplier) {
        this.errorSupplier = errorSupplier;
    }
    @Override
    public void subscribeActual(Observer<? super T> s) {
        Throwable error;
        try {
            error = ObjectHelper.requireNonNull(errorSupplier.call(), "Callable returned null throwable. Null values are generally not allowed in 2.x operators and sources.");
        } catch (Throwable t) {
            Exceptions.throwIfFatal(t);
            error = t;
        }
        EmptyDisposable.error(error, s);
    }
}
