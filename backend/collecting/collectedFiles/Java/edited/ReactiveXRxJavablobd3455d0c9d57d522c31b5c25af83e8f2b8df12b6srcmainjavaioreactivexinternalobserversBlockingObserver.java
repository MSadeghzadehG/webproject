

package io.reactivex.internal.observers;

import java.util.Queue;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observer;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;
import io.reactivex.internal.util.NotificationLite;

public final class BlockingObserver<T> extends AtomicReference<Disposable> implements Observer<T>, Disposable {

    private static final long serialVersionUID = -4875965440900746268L;

    public static final Object TERMINATED = new Object();

    final Queue<Object> queue;

    public BlockingObserver(Queue<Object> queue) {
        this.queue = queue;
    }

    @Override
    public void onSubscribe(Disposable s) {
        DisposableHelper.setOnce(this, s);
    }

    @Override
    public void onNext(T t) {
        queue.offer(NotificationLite.next(t));
    }

    @Override
    public void onError(Throwable t) {
        queue.offer(NotificationLite.error(t));
    }

    @Override
    public void onComplete() {
        queue.offer(NotificationLite.complete());
    }

    @Override
    public void dispose() {
        if (DisposableHelper.dispose(this)) {
            queue.offer(TERMINATED);
        }
    }

    @Override
    public boolean isDisposed() {
        return get() == DisposableHelper.DISPOSED;
    }
}
