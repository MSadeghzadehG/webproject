

package io.reactivex.internal.operators.observable;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BiFunction;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.operators.observable.ObservableReduceSeedSingle.ReduceSeedObserver;


public final class ObservableReduceWithSingle<T, R> extends Single<R> {

    final ObservableSource<T> source;

    final Callable<R> seedSupplier;

    final BiFunction<R, ? super T, R> reducer;

    public ObservableReduceWithSingle(ObservableSource<T> source, Callable<R> seedSupplier, BiFunction<R, ? super T, R> reducer) {
        this.source = source;
        this.seedSupplier = seedSupplier;
        this.reducer = reducer;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super R> observer) {
        R seed;

        try {
            seed = ObjectHelper.requireNonNull(seedSupplier.call(), "The seedSupplier returned a null value");
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            EmptyDisposable.error(ex, observer);
            return;
        }
        source.subscribe(new ReduceSeedObserver<T, R>(observer, reducer, seed));
    }
}
