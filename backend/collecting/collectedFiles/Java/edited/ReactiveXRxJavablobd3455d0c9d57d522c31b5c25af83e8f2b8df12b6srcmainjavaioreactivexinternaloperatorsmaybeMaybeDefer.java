

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;


public final class MaybeDefer<T> extends Maybe<T> {

    final Callable<? extends MaybeSource<? extends T>> maybeSupplier;

    public MaybeDefer(Callable<? extends MaybeSource<? extends T>> maybeSupplier) {
        this.maybeSupplier = maybeSupplier;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        MaybeSource<? extends T> source;

        try {
            source = ObjectHelper.requireNonNull(maybeSupplier.call(), "The maybeSupplier returned a null MaybeSource");
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            EmptyDisposable.error(ex, observer);
            return;
        }

        source.subscribe(observer);
    }
}
