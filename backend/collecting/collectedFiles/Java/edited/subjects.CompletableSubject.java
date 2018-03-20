

package io.reactivex.subjects;

import io.reactivex.annotations.Nullable;
import java.util.concurrent.atomic.*;

import io.reactivex.*;
import io.reactivex.annotations.CheckReturnValue;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class CompletableSubject extends Completable implements CompletableObserver {

    final AtomicReference<CompletableDisposable[]> observers;

    static final CompletableDisposable[] EMPTY = new CompletableDisposable[0];

    static final CompletableDisposable[] TERMINATED = new CompletableDisposable[0];

    final AtomicBoolean once;
    Throwable error;

    
    @CheckReturnValue
    public static CompletableSubject create() {
        return new CompletableSubject();
    }

    CompletableSubject() {
        once = new AtomicBoolean();
        observers = new AtomicReference<CompletableDisposable[]>(EMPTY);
    }

    @Override
    public void onSubscribe(Disposable d) {
        if (observers.get() == TERMINATED) {
            d.dispose();
        }
    }

    @Override
    public void onError(Throwable e) {
        ObjectHelper.requireNonNull(e, "onError called with null. Null values are generally not allowed in 2.x operators and sources.");
        if (once.compareAndSet(false, true)) {
            this.error = e;
            for (CompletableDisposable md : observers.getAndSet(TERMINATED)) {
                md.actual.onError(e);
            }
        } else {
            RxJavaPlugins.onError(e);
        }
    }

    @Override
    public void onComplete() {
        if (once.compareAndSet(false, true)) {
            for (CompletableDisposable md : observers.getAndSet(TERMINATED)) {
                md.actual.onComplete();
            }
        }
    }

    @Override
    protected void subscribeActual(CompletableObserver observer) {
        CompletableDisposable md = new CompletableDisposable(observer, this);
        observer.onSubscribe(md);
        if (add(md)) {
            if (md.isDisposed()) {
                remove(md);
            }
        } else {
            Throwable ex = error;
            if (ex != null) {
                observer.onError(ex);
            } else {
                observer.onComplete();
            }
        }
    }

    boolean add(CompletableDisposable inner) {
        for (;;) {
            CompletableDisposable[] a = observers.get();
            if (a == TERMINATED) {
                return false;
            }

            int n = a.length;

            CompletableDisposable[] b = new CompletableDisposable[n + 1];
            System.arraycopy(a, 0, b, 0, n);
            b[n] = inner;
            if (observers.compareAndSet(a, b)) {
                return true;
            }
        }
    }

    void remove(CompletableDisposable inner) {
        for (;;) {
            CompletableDisposable[] a = observers.get();
            int n = a.length;
            if (n == 0) {
                return;
            }

            int j = -1;

            for (int i = 0; i < n; i++) {
                if (a[i] == inner) {
                    j = i;
                    break;
                }
            }

            if (j < 0) {
                return;
            }
            CompletableDisposable[] b;
            if (n == 1) {
                b = EMPTY;
            } else {
                b = new CompletableDisposable[n - 1];
                System.arraycopy(a, 0, b, 0, j);
                System.arraycopy(a, j + 1, b, j, n - j - 1);
            }

            if (observers.compareAndSet(a, b)) {
                return;
            }
        }
    }

    
    @Nullable
    public Throwable getThrowable() {
        if (observers.get() == TERMINATED) {
            return error;
        }
        return null;
    }

    
    public boolean hasThrowable() {
        return observers.get() == TERMINATED && error != null;
    }

    
    public boolean hasComplete() {
        return observers.get() == TERMINATED && error == null;
    }

    
    public boolean hasObservers() {
        return observers.get().length != 0;
    }

    
     int observerCount() {
        return observers.get().length;
    }

    static final class CompletableDisposable
    extends AtomicReference<CompletableSubject> implements Disposable {
        private static final long serialVersionUID = -7650903191002190468L;

        final CompletableObserver actual;

        CompletableDisposable(CompletableObserver actual, CompletableSubject parent) {
            this.actual = actual;
            lazySet(parent);
        }

        @Override
        public void dispose() {
            CompletableSubject parent = getAndSet(null);
            if (parent != null) {
                parent.remove(this);
            }
        }

        @Override
        public boolean isDisposed() {
            return get() == null;
        }
    }
}
