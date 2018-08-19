

package io.reactivex.internal.operators.maybe;

import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;
import io.reactivex.exceptions.Exceptions;


public final class MaybeErrorCallable<T> extends Maybe<T> {

    final Callable<? extends Throwable> errorSupplier;

    public MaybeErrorCallable(Callable<? extends Throwable> errorSupplier) {
        this.errorSupplier = errorSupplier;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        observer.onSubscribe(Disposables.disposed());
        Throwable ex;

        try {
            ex = ObjectHelper.requireNonNull(errorSupplier.call(), "Callable returned null throwable. Null values are generally not allowed in 2.x operators and sources.");
        } catch (Throwable ex1) {
            Exceptions.throwIfFatal(ex1);
            ex = ex1;
        }

        observer.onError(ex);
    }
}
