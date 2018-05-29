

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;
import io.reactivex.internal.fuseable.ScalarCallable;


public final class MaybeJust<T> extends Maybe<T> implements ScalarCallable<T> {

    final T value;

    public MaybeJust(T value) {
        this.value = value;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        observer.onSubscribe(Disposables.disposed());
        observer.onSuccess(value);
    }

    @Override
    public T call() {
        return value;
    }
}
