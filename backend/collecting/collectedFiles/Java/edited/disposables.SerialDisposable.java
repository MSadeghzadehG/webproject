

package io.reactivex.disposables;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.annotations.Nullable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class SerialDisposable implements Disposable {
    final AtomicReference<Disposable> resource;

    
    public SerialDisposable() {
        this.resource = new AtomicReference<Disposable>();
    }

    
    public SerialDisposable(@Nullable Disposable initialDisposable) {
        this.resource = new AtomicReference<Disposable>(initialDisposable);
    }

    
    public boolean set(@Nullable Disposable next) {
        return DisposableHelper.set(resource, next);
    }

    
    public boolean replace(@Nullable Disposable next) {
        return DisposableHelper.replace(resource, next);
    }

    
    @Nullable
    public Disposable get() {
        Disposable d = resource.get();
        if (d == DisposableHelper.DISPOSED) {
            return Disposables.disposed();
        }
        return d;
    }

    @Override
    public void dispose() {
        DisposableHelper.dispose(resource);
    }

    @Override
    public boolean isDisposed() {
        return DisposableHelper.isDisposed(resource.get());
    }
}
