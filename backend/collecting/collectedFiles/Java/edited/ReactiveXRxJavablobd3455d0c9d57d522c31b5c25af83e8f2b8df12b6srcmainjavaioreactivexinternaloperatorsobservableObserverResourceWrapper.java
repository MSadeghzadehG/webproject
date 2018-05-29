

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;

public final class ObserverResourceWrapper<T> extends AtomicReference<Disposable> implements Observer<T>, Disposable {

    private static final long serialVersionUID = -8612022020200669122L;

    final Observer<? super T> actual;

    final AtomicReference<Disposable> subscription = new AtomicReference<Disposable>();

    public ObserverResourceWrapper(Observer<? super T> actual) {
        this.actual = actual;
    }

    @Override
    public void onSubscribe(Disposable s) {
        if (DisposableHelper.setOnce(subscription, s)) {
            actual.onSubscribe(this);
        }
    }

    @Override
    public void onNext(T t) {
        actual.onNext(t);
    }

    @Override
    public void onError(Throwable t) {
        dispose();
        actual.onError(t);
    }

    @Override
    public void onComplete() {
        dispose();
        actual.onComplete();
    }

    @Override
    public void dispose() {
        DisposableHelper.dispose(subscription);

        DisposableHelper.dispose(this);
    }

    @Override
    public boolean isDisposed() {
        return subscription.get() == DisposableHelper.DISPOSED;
    }

    public void setResource(Disposable resource) {
        DisposableHelper.set(this, resource);
    }
}
