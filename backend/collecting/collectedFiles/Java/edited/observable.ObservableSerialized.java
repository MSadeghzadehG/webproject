
package io.reactivex.internal.operators.observable;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.observers.SerializedObserver;

public final class ObservableSerialized<T> extends AbstractObservableWithUpstream<T, T> {
    public ObservableSerialized(Observable<T> upstream) {
        super(upstream);
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(new SerializedObserver<T>(observer));
    }
}
