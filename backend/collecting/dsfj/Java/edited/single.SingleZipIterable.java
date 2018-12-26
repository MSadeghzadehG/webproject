

package io.reactivex.internal.operators.single;

import java.util.*;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.operators.single.SingleZipArray.ZipCoordinator;

public final class SingleZipIterable<T, R> extends Single<R> {

    final Iterable<? extends SingleSource<? extends T>> sources;

    final Function<? super Object[], ? extends R> zipper;

    public SingleZipIterable(Iterable<? extends SingleSource<? extends T>> sources, Function<? super Object[], ? extends R> zipper) {
        this.sources = sources;
        this.zipper = zipper;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super R> observer) {
        @SuppressWarnings("unchecked")
        SingleSource<? extends T>[] a = new SingleSource[8];
        int n = 0;

        try {
            for (SingleSource<? extends T> source : sources) {
                if (source == null) {
                    EmptyDisposable.error(new NullPointerException("One of the sources is null"), observer);
                    return;
                }
                if (n == a.length) {
                    a = Arrays.copyOf(a, n + (n >> 2));
                }
                a[n++] = source;
            }
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            EmptyDisposable.error(ex, observer);
            return;
        }

        if (n == 0) {
            EmptyDisposable.error(new NoSuchElementException(), observer);
            return;
        }

        if (n == 1) {
            a[0].subscribe(new SingleMap.MapSingleObserver<T, R>(observer, new SingletonArrayFunc()));
            return;
        }

        ZipCoordinator<T, R> parent = new ZipCoordinator<T, R>(observer, n, zipper);

        observer.onSubscribe(parent);

        for (int i = 0; i < n; i++) {
            if (parent.isDisposed()) {
                return;
            }

            a[i].subscribe(parent.observers[i]);
        }
    }

    final class SingletonArrayFunc implements Function<T, R> {
        @Override
        public R apply(T t) throws Exception {
            return ObjectHelper.requireNonNull(zipper.apply(new Object[] { t }), "The zipper returned a null value");
        }
    }
}
