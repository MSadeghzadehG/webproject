

package io.reactivex.internal.observers;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.*;
import io.reactivex.internal.util.QueueDrainHelper;


public final class InnerQueuedObserver<T>
extends AtomicReference<Disposable>
implements Observer<T>, Disposable {


    private static final long serialVersionUID = -5417183359794346637L;

    final InnerQueuedObserverSupport<T> parent;

    final int prefetch;

    SimpleQueue<T> queue;

    volatile boolean done;

    int fusionMode;

    public InnerQueuedObserver(InnerQueuedObserverSupport<T> parent, int prefetch) {
        this.parent = parent;
        this.prefetch = prefetch;
    }

    @Override
    public void onSubscribe(Disposable s) {
        if (DisposableHelper.setOnce(this, s)) {
            if (s instanceof QueueDisposable) {
                @SuppressWarnings("unchecked")
                QueueDisposable<T> qs = (QueueDisposable<T>) s;

                int m = qs.requestFusion(QueueDisposable.ANY);
                if (m == QueueSubscription.SYNC) {
                    fusionMode = m;
                    queue = qs;
                    done = true;
                    parent.innerComplete(this);
                    return;
                }
                if (m == QueueDisposable.ASYNC) {
                    fusionMode = m;
                    queue = qs;
                    return;
                }
            }

            queue = QueueDrainHelper.createQueue(-prefetch);
        }
    }

    @Override
    public void onNext(T t) {
        if (fusionMode == QueueDisposable.NONE) {
            parent.innerNext(this, t);
        } else {
            parent.drain();
        }
    }

    @Override
    public void onError(Throwable t) {
        parent.innerError(this, t);
    }

    @Override
    public void onComplete() {
        parent.innerComplete(this);
    }

    @Override
    public void dispose() {
        DisposableHelper.dispose(this);
    }

    @Override
    public boolean isDisposed() {
        return DisposableHelper.isDisposed(get());
    }

    public boolean isDone() {
        return done;
    }

    public void setDone() {
        this.done = true;
    }

    public SimpleQueue<T> queue() {
        return queue;
    }

    public int fusionMode() {
        return fusionMode;
    }
}
