

package io.reactivex.internal.observers;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public abstract class DeferredScalarObserver<T, R>
extends DeferredScalarDisposable<R>
implements Observer<T> {

    private static final long serialVersionUID = -266195175408988651L;

    
    protected Disposable s;

    
    public DeferredScalarObserver(Observer<? super R> actual) {
        super(actual);
    }

    @Override
    public void onSubscribe(Disposable s) {
        if (DisposableHelper.validate(this.s, s)) {
            this.s = s;

            actual.onSubscribe(this);
        }
    }

    @Override
    public void onError(Throwable t) {
        value = null;
        error(t);
    }

    @Override
    public void onComplete() {
        R v = value;
        if (v != null) {
            value = null;
            complete(v);
        } else {
            complete();
        }
    }

    @Override
    public void dispose() {
        super.dispose();
        s.dispose();
    }
}
