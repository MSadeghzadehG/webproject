

package io.reactivex.internal.operators.observable;

import io.reactivex.*;

public final class ObservableFromUnsafeSource<T> extends Observable<T> {
    final ObservableSource<T> source;

    public ObservableFromUnsafeSource(ObservableSource<T> source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(observer);
    }
}
