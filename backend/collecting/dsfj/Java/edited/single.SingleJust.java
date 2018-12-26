

package io.reactivex.internal.operators.single;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;

public final class SingleJust<T> extends Single<T> {

    final T value;

    public SingleJust(T value) {
        this.value = value;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> s) {
        s.onSubscribe(Disposables.disposed());
        s.onSuccess(value);
    }

}
