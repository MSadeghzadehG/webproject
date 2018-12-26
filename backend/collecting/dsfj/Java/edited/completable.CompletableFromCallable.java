

package io.reactivex.internal.operators.completable;

import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.Exceptions;

public final class CompletableFromCallable extends Completable {

    final Callable<?> callable;

    public CompletableFromCallable(Callable<?> callable) {
        this.callable = callable;
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        Disposable d = Disposables.empty();
        s.onSubscribe(d);
        try {
            callable.call();
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            if (!d.isDisposed()) {
                s.onError(e);
            }
            return;
        }
        if (!d.isDisposed()) {
            s.onComplete();
        }
    }
}
