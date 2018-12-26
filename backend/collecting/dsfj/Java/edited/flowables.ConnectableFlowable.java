

package io.reactivex.flowables;

import io.reactivex.annotations.NonNull;
import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.operators.flowable.*;
import io.reactivex.internal.util.ConnectConsumer;
import io.reactivex.plugins.RxJavaPlugins;


public abstract class ConnectableFlowable<T> extends Flowable<T> {

    
    public abstract void connect(@NonNull Consumer<? super Disposable> connection);

    
    public final Disposable connect() {
        ConnectConsumer cc = new ConnectConsumer();
        connect(cc);
        return cc.disposable;
    }

    
    @NonNull
    public Flowable<T> refCount() {
        return RxJavaPlugins.onAssembly(new FlowableRefCount<T>(this));
    }

    
    @NonNull
    public Flowable<T> autoConnect() {
        return autoConnect(1);
    }
    
    @NonNull
    public Flowable<T> autoConnect(int numberOfSubscribers) {
        return autoConnect(numberOfSubscribers, Functions.emptyConsumer());
    }

    
    @NonNull
    public Flowable<T> autoConnect(int numberOfSubscribers, @NonNull Consumer<? super Disposable> connection) {
        if (numberOfSubscribers <= 0) {
            this.connect(connection);
            return RxJavaPlugins.onAssembly(this);
        }
        return RxJavaPlugins.onAssembly(new FlowableAutoConnect<T>(this, numberOfSubscribers, connection));
    }
}
