

package io.reactivex.disposables;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.annotations.NonNull;
import io.reactivex.internal.functions.ObjectHelper;


abstract class ReferenceDisposable<T> extends AtomicReference<T> implements Disposable {

    private static final long serialVersionUID = 6537757548749041217L;

    ReferenceDisposable(T value) {
        super(ObjectHelper.requireNonNull(value, "value is null"));
    }

    protected abstract void onDisposed(@NonNull T value);

    @Override
    public final void dispose() {
        T value = get();
        if (value != null) {
            value = getAndSet(null);
            if (value != null) {
                onDisposed(value);
            }
        }
    }

    @Override
    public final boolean isDisposed() {
        return get() == null;
    }
}
