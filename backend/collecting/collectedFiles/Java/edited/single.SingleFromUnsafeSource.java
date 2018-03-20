

package io.reactivex.internal.operators.single;

import io.reactivex.*;

public final class SingleFromUnsafeSource<T> extends Single<T> {
    final SingleSource<T> source;

    public SingleFromUnsafeSource(SingleSource<T> source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> observer) {
        source.subscribe(observer);
    }
}
