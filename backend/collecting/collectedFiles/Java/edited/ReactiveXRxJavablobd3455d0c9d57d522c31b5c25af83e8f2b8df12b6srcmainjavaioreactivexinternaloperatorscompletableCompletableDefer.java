

package io.reactivex.internal.operators.completable;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;

public final class CompletableDefer extends Completable {

    final Callable<? extends CompletableSource> completableSupplier;

    public CompletableDefer(Callable<? extends CompletableSource> completableSupplier) {
        this.completableSupplier = completableSupplier;
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        CompletableSource c;

        try {
            c = ObjectHelper.requireNonNull(completableSupplier.call(), "The completableSupplier returned a null CompletableSource");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            EmptyDisposable.error(e, s);
            return;
        }

        c.subscribe(s);
    }

}
