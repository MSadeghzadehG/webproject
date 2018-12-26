

package io.reactivex.internal.operators.completable;

import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;

public final class CompletableErrorSupplier extends Completable {

    final Callable<? extends Throwable> errorSupplier;

    public CompletableErrorSupplier(Callable<? extends Throwable> errorSupplier) {
        this.errorSupplier = errorSupplier;
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        Throwable error;

        try {
            error = ObjectHelper.requireNonNull(errorSupplier.call(), "The error returned is null");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            error = e;
        }

        EmptyDisposable.error(error, s);
    }

}
