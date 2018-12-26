

package io.reactivex.internal.observers;

import io.reactivex.Observer;
import io.reactivex.annotations.Nullable;
import io.reactivex.plugins.RxJavaPlugins;


public class DeferredScalarDisposable<T> extends BasicIntQueueDisposable<T> {

    private static final long serialVersionUID = -5502432239815349361L;

    
    protected final Observer<? super T> actual;

    
    protected T value;

    
    static final int TERMINATED = 2;

    
    static final int DISPOSED = 4;

    
    static final int FUSED_EMPTY = 8;
    
    static final int FUSED_READY = 16;
    
    static final int FUSED_CONSUMED = 32;

    
    public DeferredScalarDisposable(Observer<? super T> actual) {
        this.actual = actual;
    }

    @Override
    public final int requestFusion(int mode) {
        if ((mode & ASYNC) != 0) {
            lazySet(FUSED_EMPTY);
            return ASYNC;
        }
        return NONE;
    }

    
    public final void complete(T value) {
        int state = get();
        if ((state & (FUSED_READY | FUSED_CONSUMED | TERMINATED | DISPOSED)) != 0) {
            return;
        }
        Observer<? super T> a = actual;
        if (state == FUSED_EMPTY) {
            this.value = value;
            lazySet(FUSED_READY);
            a.onNext(null);
        } else {
            lazySet(TERMINATED);
            a.onNext(value);
        }
        if (get() != DISPOSED) {
            a.onComplete();
        }
    }

    
     public final void error(Throwable t) {
        int state = get();
        if ((state & (FUSED_READY | FUSED_CONSUMED | TERMINATED | DISPOSED)) != 0) {
            RxJavaPlugins.onError(t);
            return;
        }
        lazySet(TERMINATED);
        actual.onError(t);
    }

     
    public final void complete() {
        int state = get();
        if ((state & (FUSED_READY | FUSED_CONSUMED | TERMINATED | DISPOSED)) != 0) {
            return;
        }
        lazySet(TERMINATED);
        actual.onComplete();
    }

    @Nullable
    @Override
    public final T poll() throws Exception {
        if (get() == FUSED_READY) {
            T v = value;
            value = null;
            lazySet(FUSED_CONSUMED);
            return v;
        }
        return null;
    }

    @Override
    public final boolean isEmpty() {
        return get() != FUSED_READY;
    }

    @Override
    public final void clear() {
        lazySet(FUSED_CONSUMED);
        value = null;
    }

    @Override
    public void dispose() {
        set(DISPOSED);
        value = null;
    }

    
    public final boolean tryDispose() {
        return getAndSet(DISPOSED) != DISPOSED;
    }

    @Override
    public final boolean isDisposed() {
        return get() == DISPOSED;
    }

}
