
package io.reactivex.internal.observers;

import java.util.concurrent.CountDownLatch;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.util.*;

public abstract class BlockingBaseObserver<T> extends CountDownLatch
implements Observer<T>, Disposable {

    T value;
    Throwable error;

    Disposable d;

    volatile boolean cancelled;

    public BlockingBaseObserver() {
        super(1);
    }

    @Override
    public final void onSubscribe(Disposable d) {
        this.d = d;
        if (cancelled) {
            d.dispose();
        }
    }

    @Override
    public final void onComplete() {
        countDown();
    }

    @Override
    public final void dispose() {
        cancelled = true;
        Disposable d = this.d;
        if (d != null) {
            d.dispose();
        }
    }

    @Override
    public final boolean isDisposed() {
        return cancelled;
    }

    
    public final T blockingGet() {
        if (getCount() != 0) {
            try {
                BlockingHelper.verifyNonBlocking();
                await();
            } catch (InterruptedException ex) {
                dispose();
                throw ExceptionHelper.wrapOrThrow(ex);
            }
        }

        Throwable e = error;
        if (e != null) {
            throw ExceptionHelper.wrapOrThrow(e);
        }
        return value;
    }
}
