

package io.reactivex.internal.operators.observable;

import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;

public final class ObservableDefer<T> extends Observable<T> {
    final Callable<? extends ObservableSource<? extends T>> supplier;
    public ObservableDefer(Callable<? extends ObservableSource<? extends T>> supplier) {
        this.supplier = supplier;
    }
    @Override
    public void subscribeActual(Observer<? super T> s) {
        ObservableSource<? extends T> pub;
        try {
            pub = ObjectHelper.requireNonNull(supplier.call(), "null ObservableSource supplied");
        } catch (Throwable t) {
            Exceptions.throwIfFatal(t);
            EmptyDisposable.error(t, s);
            return;
        }

        pub.subscribe(s);
    }
}
