

package io.reactivex.internal.observers;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.QueueDisposable;
import io.reactivex.plugins.RxJavaPlugins;


public abstract class BasicFuseableObserver<T, R> implements Observer<T>, QueueDisposable<R> {

    
    protected final Observer<? super R> actual;

    
    protected Disposable s;

    
    protected QueueDisposable<T> qs;

    
    protected boolean done;

    
    protected int sourceMode;

    
    public BasicFuseableObserver(Observer<? super R> actual) {
        this.actual = actual;
    }

        @SuppressWarnings("unchecked")
    @Override
    public final void onSubscribe(Disposable s) {
        if (DisposableHelper.validate(this.s, s)) {

            this.s = s;
            if (s instanceof QueueDisposable) {
                this.qs = (QueueDisposable<T>)s;
            }

            if (beforeDownstream()) {

                actual.onSubscribe(this);

                afterDownstream();
            }

        }
    }

    
    protected boolean beforeDownstream() {
        return true;
    }

    
    protected void afterDownstream() {
            }

            
    @Override
    public void onError(Throwable t) {
        if (done) {
            RxJavaPlugins.onError(t);
            return;
        }
        done = true;
        actual.onError(t);
    }

    
    protected final void fail(Throwable t) {
        Exceptions.throwIfFatal(t);
        s.dispose();
        onError(t);
    }

    @Override
    public void onComplete() {
        if (done) {
            return;
        }
        done = true;
        actual.onComplete();
    }

    
    protected final int transitiveBoundaryFusion(int mode) {
        QueueDisposable<T> qs = this.qs;
        if (qs != null) {
            if ((mode & BOUNDARY) == 0) {
                int m = qs.requestFusion(mode);
                if (m != NONE) {
                    sourceMode = m;
                }
                return m;
            }
        }
        return NONE;
    }

            
    @Override
    public void dispose() {
        s.dispose();
    }

    @Override
    public boolean isDisposed() {
        return s.isDisposed();
    }

    @Override
    public boolean isEmpty() {
        return qs.isEmpty();
    }

    @Override
    public void clear() {
        qs.clear();
    }

            
    @Override
    public final boolean offer(R e) {
        throw new UnsupportedOperationException("Should not be called!");
    }

    @Override
    public final boolean offer(R v1, R v2) {
        throw new UnsupportedOperationException("Should not be called!");
    }
}
