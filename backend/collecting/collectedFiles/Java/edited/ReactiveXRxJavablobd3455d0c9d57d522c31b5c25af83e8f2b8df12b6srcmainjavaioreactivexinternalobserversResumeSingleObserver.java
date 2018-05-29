

package io.reactivex.internal.observers;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.SingleObserver;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class ResumeSingleObserver<T> implements SingleObserver<T> {

    final AtomicReference<Disposable> parent;

    final SingleObserver<? super T> actual;

    public ResumeSingleObserver(AtomicReference<Disposable> parent, SingleObserver<? super T> actual) {
        this.parent = parent;
        this.actual = actual;
    }

    @Override
    public void onSubscribe(Disposable d) {
        DisposableHelper.replace(parent, d);
    }

    @Override
    public void onSuccess(T value) {
        actual.onSuccess(value);
    }

    @Override
    public void onError(Throwable e) {
        actual.onError(e);
    }
}
