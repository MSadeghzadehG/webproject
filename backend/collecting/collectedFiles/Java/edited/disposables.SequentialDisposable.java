

package io.reactivex.internal.disposables;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.disposables.Disposable;


public final class SequentialDisposable
extends AtomicReference<Disposable>
implements Disposable {


    private static final long serialVersionUID = -754898800686245608L;

    
    public SequentialDisposable() {
            }

    
    public SequentialDisposable(Disposable initial) {
        lazySet(initial);
    }

    
    public boolean update(Disposable next) {
        return DisposableHelper.set(this, next);
    }

    
    public boolean replace(Disposable next) {
        return DisposableHelper.replace(this, next);
    }

    @Override
    public void dispose() {
        DisposableHelper.dispose(this);
    }

    @Override
    public boolean isDisposed() {
        return DisposableHelper.isDisposed(get());
    }
}
