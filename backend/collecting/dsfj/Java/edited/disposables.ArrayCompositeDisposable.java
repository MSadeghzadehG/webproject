

package io.reactivex.internal.disposables;

import java.util.concurrent.atomic.AtomicReferenceArray;

import io.reactivex.disposables.Disposable;


public final class ArrayCompositeDisposable extends AtomicReferenceArray<Disposable> implements Disposable {

    private static final long serialVersionUID = 2746389416410565408L;

    public ArrayCompositeDisposable(int capacity) {
        super(capacity);
    }

    
    public boolean setResource(int index, Disposable resource) {
        for (;;) {
            Disposable o = get(index);
            if (o == DisposableHelper.DISPOSED) {
                resource.dispose();
                return false;
            }
            if (compareAndSet(index, o, resource)) {
                if (o != null) {
                    o.dispose();
                }
                return true;
            }
        }
    }

    
    public Disposable replaceResource(int index, Disposable resource) {
        for (;;) {
            Disposable o = get(index);
            if (o == DisposableHelper.DISPOSED) {
                resource.dispose();
                return null;
            }
            if (compareAndSet(index, o, resource)) {
                return o;
            }
        }
    }

    @Override
    public void dispose() {
        if (get(0) != DisposableHelper.DISPOSED) {
            int s = length();
            for (int i = 0; i < s; i++) {
                Disposable o = get(i);
                if (o != DisposableHelper.DISPOSED) {
                    o = getAndSet(i, DisposableHelper.DISPOSED);
                    if (o != DisposableHelper.DISPOSED && o != null) {
                        o.dispose();
                    }
                }
            }
        }
    }

    @Override
    public boolean isDisposed() {
        return get(0) == DisposableHelper.DISPOSED;
    }
}
