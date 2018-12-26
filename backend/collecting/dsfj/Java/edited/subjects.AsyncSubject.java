

package io.reactivex.subjects;

import io.reactivex.annotations.Nullable;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observer;
import io.reactivex.annotations.CheckReturnValue;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.observers.DeferredScalarDisposable;
import io.reactivex.plugins.RxJavaPlugins;


public final class AsyncSubject<T> extends Subject<T> {

    @SuppressWarnings("rawtypes")
    static final AsyncDisposable[] EMPTY = new AsyncDisposable[0];

    @SuppressWarnings("rawtypes")
    static final AsyncDisposable[] TERMINATED = new AsyncDisposable[0];

    final AtomicReference<AsyncDisposable<T>[]> subscribers;

    
    Throwable error;

    
    T value;

    
    @CheckReturnValue
    public static <T> AsyncSubject<T> create() {
        return new AsyncSubject<T>();
    }

    
    @SuppressWarnings("unchecked")
    AsyncSubject() {
        this.subscribers = new AtomicReference<AsyncDisposable<T>[]>(EMPTY);
    }

    @Override
    public void onSubscribe(Disposable s) {
        if (subscribers.get() == TERMINATED) {
            s.dispose();
        }
    }

    @Override
    public void onNext(T t) {
        ObjectHelper.requireNonNull(t, "onNext called with null. Null values are generally not allowed in 2.x operators and sources.");
        if (subscribers.get() == TERMINATED) {
            return;
        }
        value = t;
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onError(Throwable t) {
        ObjectHelper.requireNonNull(t, "onError called with null. Null values are generally not allowed in 2.x operators and sources.");
        if (subscribers.get() == TERMINATED) {
            RxJavaPlugins.onError(t);
            return;
        }
        value = null;
        error = t;
        for (AsyncDisposable<T> as : subscribers.getAndSet(TERMINATED)) {
            as.onError(t);
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onComplete() {
        if (subscribers.get() == TERMINATED) {
            return;
        }
        T v = value;
        AsyncDisposable<T>[] array = subscribers.getAndSet(TERMINATED);
        if (v == null) {
            for (AsyncDisposable<T> as : array) {
                as.onComplete();
            }
        } else {
            for (AsyncDisposable<T> as : array) {
                as.complete(v);
            }
        }
    }

    @Override
    public boolean hasObservers() {
        return subscribers.get().length != 0;
    }

    @Override
    public boolean hasThrowable() {
        return subscribers.get() == TERMINATED && error != null;
    }

    @Override
    public boolean hasComplete() {
        return subscribers.get() == TERMINATED && error == null;
    }

    @Override
    public Throwable getThrowable() {
        return subscribers.get() == TERMINATED ? error : null;
    }

    @Override
    protected void subscribeActual(Observer<? super T> s) {
        AsyncDisposable<T> as = new AsyncDisposable<T>(s, this);
        s.onSubscribe(as);
        if (add(as)) {
            if (as.isDisposed()) {
                remove(as);
            }
        } else {
            Throwable ex = error;
            if (ex != null) {
                s.onError(ex);
            } else {
                T v = value;
                if (v != null) {
                    as.complete(v);
                } else {
                    as.onComplete();
                }
            }
        }
    }

    
    boolean add(AsyncDisposable<T> ps) {
        for (;;) {
            AsyncDisposable<T>[] a = subscribers.get();
            if (a == TERMINATED) {
                return false;
            }

            int n = a.length;
            @SuppressWarnings("unchecked")
            AsyncDisposable<T>[] b = new AsyncDisposable[n + 1];
            System.arraycopy(a, 0, b, 0, n);
            b[n] = ps;

            if (subscribers.compareAndSet(a, b)) {
                return true;
            }
        }
    }

    
    @SuppressWarnings("unchecked")
    void remove(AsyncDisposable<T> ps) {
        for (;;) {
            AsyncDisposable<T>[] a = subscribers.get();
            int n = a.length;
            if (n == 0) {
                return;
            }

            int j = -1;
            for (int i = 0; i < n; i++) {
                if (a[i] == ps) {
                    j = i;
                    break;
                }
            }

            if (j < 0) {
                return;
            }

            AsyncDisposable<T>[] b;

            if (n == 1) {
                b = EMPTY;
            } else {
                b = new AsyncDisposable[n - 1];
                System.arraycopy(a, 0, b, 0, j);
                System.arraycopy(a, j + 1, b, j, n - j - 1);
            }
            if (subscribers.compareAndSet(a, b)) {
                return;
            }
        }
    }

    
    public boolean hasValue() {
        return subscribers.get() == TERMINATED && value != null;
    }

    
    @Nullable
    public T getValue() {
        return subscribers.get() == TERMINATED ? value : null;
    }

    
    public Object[] getValues() {
        T v = getValue();
        return v != null ? new Object[] { v } : new Object[0];
    }

    
    public T[] getValues(T[] array) {
        T v = getValue();
        if (v == null) {
            if (array.length != 0) {
                array[0] = null;
            }
            return array;
        }
        if (array.length == 0) {
            array = Arrays.copyOf(array, 1);
        }
        array[0] = v;
        if (array.length != 1) {
            array[1] = null;
        }
        return array;
    }

    static final class AsyncDisposable<T> extends DeferredScalarDisposable<T> {
        private static final long serialVersionUID = 5629876084736248016L;

        final AsyncSubject<T> parent;

        AsyncDisposable(Observer<? super T> actual, AsyncSubject<T> parent) {
            super(actual);
            this.parent = parent;
        }

        @Override
        public void dispose() {
            if (super.tryDispose()) {
                parent.remove(this);
            }
        }

        void onComplete() {
            if (!isDisposed()) {
                actual.onComplete();
            }
        }

        void onError(Throwable t) {
            if (isDisposed()) {
                RxJavaPlugins.onError(t);
            } else {
                actual.onError(t);
            }
        }
    }
}
