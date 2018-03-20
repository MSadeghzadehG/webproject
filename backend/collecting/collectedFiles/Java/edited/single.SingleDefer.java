

package io.reactivex.internal.operators.single;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;

public final class SingleDefer<T> extends Single<T> {

    final Callable<? extends SingleSource<? extends T>> singleSupplier;

    public SingleDefer(Callable<? extends SingleSource<? extends T>> singleSupplier) {
        this.singleSupplier = singleSupplier;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> s) {
        SingleSource<? extends T> next;

        try {
            next = ObjectHelper.requireNonNull(singleSupplier.call(), "The singleSupplier returned a null SingleSource");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            EmptyDisposable.error(e, s);
            return;
        }

        next.subscribe(s);
    }

}
