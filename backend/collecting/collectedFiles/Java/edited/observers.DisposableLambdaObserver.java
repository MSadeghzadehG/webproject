

package io.reactivex.internal.observers;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.*;
import io.reactivex.internal.disposables.*;
import io.reactivex.plugins.RxJavaPlugins;

public final class DisposableLambdaObserver<T> implements Observer<T>, Disposable {
    final Observer<? super T> actual;
    final Consumer<? super Disposable> onSubscribe;
    final Action onDispose;

    Disposable s;

    public DisposableLambdaObserver(Observer<? super T> actual,
            Consumer<? super Disposable> onSubscribe,
            Action onDispose) {
        this.actual = actual;
        this.onSubscribe = onSubscribe;
        this.onDispose = onDispose;
    }

    @Override
    public void onSubscribe(Disposable s) {
                try {
            onSubscribe.accept(s);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            s.dispose();
            this.s = DisposableHelper.DISPOSED;
            EmptyDisposable.error(e, actual);
            return;
        }
        if (DisposableHelper.validate(this.s, s)) {
            this.s = s;
            actual.onSubscribe(this);
        }
    }

    @Override
    public void onNext(T t) {
        actual.onNext(t);
    }

    @Override
    public void onError(Throwable t) {
        if (s != DisposableHelper.DISPOSED) {
            actual.onError(t);
        } else {
            RxJavaPlugins.onError(t);
        }
    }

    @Override
    public void onComplete() {
        if (s != DisposableHelper.DISPOSED) {
            actual.onComplete();
        }
    }


    @Override
    public void dispose() {
        try {
            onDispose.run();
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            RxJavaPlugins.onError(e);
        }
        s.dispose();
    }

    @Override
    public boolean isDisposed() {
        return s.isDisposed();
    }
}
