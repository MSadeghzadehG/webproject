

package io.reactivex.internal.disposables;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Cancellable;
import io.reactivex.plugins.RxJavaPlugins;


public final class CancellableDisposable extends AtomicReference<Cancellable>
implements Disposable {


    private static final long serialVersionUID = 5718521705281392066L;

    public CancellableDisposable(Cancellable cancellable) {
        super(cancellable);
    }

    @Override
    public boolean isDisposed() {
        return get() == null;
    }

    @Override
    public void dispose() {
        if (get() != null) {
            Cancellable c = getAndSet(null);
            if (c != null) {
                try {
                    c.cancel();
                } catch (Exception ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
            }
        }
    }
}
