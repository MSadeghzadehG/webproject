

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicInteger;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.disposables.Disposable;
import io.reactivex.flowables.ConnectableFlowable;
import io.reactivex.functions.Consumer;


public final class FlowableAutoConnect<T> extends Flowable<T> {
    final ConnectableFlowable<? extends T> source;
    final int numberOfSubscribers;
    final Consumer<? super Disposable> connection;
    final AtomicInteger clients;

    public FlowableAutoConnect(ConnectableFlowable<? extends T> source,
            int numberOfSubscribers,
            Consumer<? super Disposable> connection) {
        this.source = source;
        this.numberOfSubscribers = numberOfSubscribers;
        this.connection = connection;
        this.clients = new AtomicInteger();
    }

    @Override
    public void subscribeActual(Subscriber<? super T> child) {
        source.subscribe(child);
        if (clients.incrementAndGet() == numberOfSubscribers) {
            source.connect(connection);
        }
    }
}
