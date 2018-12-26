

package io.reactivex.observables;

import io.reactivex.annotations.NonNull;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.operators.observable.*;
import io.reactivex.internal.util.ConnectConsumer;
import io.reactivex.plugins.RxJavaPlugins;


public abstract class ConnectableObservable<T> extends Observable<T> {

    
    public abstract void connect(@NonNull Consumer<? super Disposable> connection);

    
    public final Disposable connect() {
        ConnectConsumer cc = new ConnectConsumer();
        connect(cc);
        return cc.disposable;
    }

    
    @NonNull
    public Observable<T> refCount() {
        return RxJavaPlugins.onAssembly(new ObservableRefCount<T>(this));
    }

    
    @NonNull
    public Observable<T> autoConnect() {
        return autoConnect(1);
    }

    
    @NonNull
    public Observable<T> autoConnect(int numberOfSubscribers) {
        return autoConnect(numberOfSubscribers, Functions.emptyConsumer());
    }

    
    @NonNull
    public Observable<T> autoConnect(int numberOfSubscribers, @NonNull Consumer<? super Disposable> connection) {
        if (numberOfSubscribers <= 0) {
            this.connect(connection);
            return RxJavaPlugins.onAssembly(this);
        }
        return RxJavaPlugins.onAssembly(new ObservableAutoConnect<T>(this, numberOfSubscribers, connection));
    }
}
