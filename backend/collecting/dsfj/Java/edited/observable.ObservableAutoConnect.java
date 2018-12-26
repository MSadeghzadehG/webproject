

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.observables.ConnectableObservable;


public final class ObservableAutoConnect<T> extends Observable<T> {
    final ConnectableObservable<? extends T> source;
    final int numberOfObservers;
    final Consumer<? super Disposable> connection;
    final AtomicInteger clients;

    public ObservableAutoConnect(ConnectableObservable<? extends T> source,
            int numberOfObservers,
            Consumer<? super Disposable> connection) {
        this.source = source;
        this.numberOfObservers = numberOfObservers;
        this.connection = connection;
        this.clients = new AtomicInteger();
    }

    @Override
    public void subscribeActual(Observer<? super T> child) {
        source.subscribe(child);
        if (clients.incrementAndGet() == numberOfObservers) {
            source.connect(connection);
        }
    }
}
