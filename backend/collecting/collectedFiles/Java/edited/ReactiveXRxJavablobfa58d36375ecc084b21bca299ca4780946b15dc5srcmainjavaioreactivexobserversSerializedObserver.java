
package io.reactivex.observers;

import io.reactivex.Observer;
import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.util.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class SerializedObserver<T> implements Observer<T>, Disposable {
    final Observer<? super T> actual;
    final boolean delayError;

    static final int QUEUE_LINK_SIZE = 4;

    Disposable s;

    boolean emitting;
    AppendOnlyLinkedArrayList<Object> queue;

    volatile boolean done;

    
    public SerializedObserver(@NonNull Observer<? super T> actual) {
        this(actual, false);
    }

    
    public SerializedObserver(@NonNull Observer<? super T> actual, boolean delayError) {
        this.actual = actual;
        this.delayError = delayError;
    }

    @Override
    public void onSubscribe(@NonNull Disposable s) {
        if (DisposableHelper.validate(this.s, s)) {
            this.s = s;

            actual.onSubscribe(this);
        }
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
    public void onNext(@NonNull T t) {
        if (done) {
            return;
        }
        if (t == null) {
            s.dispose();
            onError(new NullPointerException("onNext called with null. Null values are generally not allowed in 2.x operators and sources."));
            return;
        }
        synchronized (this) {
            if (done) {
                return;
            }
            if (emitting) {
                AppendOnlyLinkedArrayList<Object> q = queue;
                if (q == null) {
                    q = new AppendOnlyLinkedArrayList<Object>(QUEUE_LINK_SIZE);
                    queue = q;
                }
                q.add(NotificationLite.next(t));
                return;
            }
            emitting = true;
        }

        actual.onNext(t);

        emitLoop();
    }

    @Override
    public void onError(@NonNull Throwable t) {
        if (done) {
            RxJavaPlugins.onError(t);
            return;
        }
        boolean reportError;
        synchronized (this) {
            if (done) {
                reportError = true;
            } else
            if (emitting) {
                done = true;
                AppendOnlyLinkedArrayList<Object> q = queue;
                if (q == null) {
                    q = new AppendOnlyLinkedArrayList<Object>(QUEUE_LINK_SIZE);
                    queue = q;
                }
                Object err = NotificationLite.error(t);
                if (delayError) {
                    q.add(err);
                } else {
                    q.setFirst(err);
                }
                return;
            } else {
                done = true;
                emitting = true;
                reportError = false;
            }
        }

        if (reportError) {
            RxJavaPlugins.onError(t);
            return;
        }

        actual.onError(t);
            }

    @Override
    public void onComplete() {
        if (done) {
            return;
        }
        synchronized (this) {
            if (done) {
                return;
            }
            if (emitting) {
                AppendOnlyLinkedArrayList<Object> q = queue;
                if (q == null) {
                    q = new AppendOnlyLinkedArrayList<Object>(QUEUE_LINK_SIZE);
                    queue = q;
                }
                q.add(NotificationLite.complete());
                return;
            }
            done = true;
            emitting = true;
        }

        actual.onComplete();
            }

    void emitLoop() {
        for (;;) {
            AppendOnlyLinkedArrayList<Object> q;
            synchronized (this) {
                q = queue;
                if (q == null) {
                    emitting = false;
                    return;
                }
                queue = null;
            }

            if (q.accept(actual)) {
                return;
            }
        }
    }
}
