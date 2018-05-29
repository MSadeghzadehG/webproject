

package io.reactivex.subjects;

import java.util.concurrent.atomic.*;

import io.reactivex.*;
import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class SingleSubject<T> extends Single<T> implements SingleObserver<T> {

    final AtomicReference<SingleDisposable<T>[]> observers;

    @SuppressWarnings("rawtypes")
    static final SingleDisposable[] EMPTY = new SingleDisposable[0];

    @SuppressWarnings("rawtypes")
    static final SingleDisposable[] TERMINATED = new SingleDisposable[0];

    final AtomicBoolean once;
    T value;
    Throwable error;

    
    @CheckReturnValue
    @NonNull
    public static <T> SingleSubject<T> create() {
        return new SingleSubject<T>();
    }

    @SuppressWarnings("unchecked")
    SingleSubject() {
        once = new AtomicBoolean();
        observers = new AtomicReference<SingleDisposable<T>[]>(EMPTY);
    }

    @Override
    public void onSubscribe(@NonNull Disposable d) {
        if (observers.get() == TERMINATED) {
            d.dispose();
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onSuccess(@NonNull T value) {
        ObjectHelper.requireNonNull(value, "onSuccess called with null. Null values are generally not allowed in 2.x operators and sources.");
        if (once.compareAndSet(false, true)) {
            this.value = value;
            for (SingleDisposable<T> md : observers.getAndSet(TERMINATED)) {
                md.actual.onSuccess(value);
            }
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onError(@NonNull Throwable e) {
        ObjectHelper.requireNonNull(e, "onError called with null. Null values are generally not allowed in 2.x operators and sources.");
        if (once.compareAndSet(false, true)) {
            this.error = e;
            for (SingleDisposable<T> md : observers.getAndSet(TERMINATED)) {
                md.actual.onError(e);
            }
        } else {
            RxJavaPlugins.onError(e);
        }
    }

    @Override
    protected void subscribeActual(@NonNull SingleObserver<? super T> observer) {
        SingleDisposable<T> md = new SingleDisposable<T>(observer, this);
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
                observer.onSuccess(value);
            }
        }
    }

    boolean add(@NonNull SingleDisposable<T> inner) {
        for (;;) {
            SingleDisposable<T>[] a = observers.get();
            if (a == TERMINATED) {
                return false;
            }

            int n = a.length;
            @SuppressWarnings("unchecked")
            SingleDisposable<T>[] b = new SingleDisposable[n + 1];
            System.arraycopy(a, 0, b, 0, n);
            b[n] = inner;
            if (observers.compareAndSet(a, b)) {
                return true;
            }
        }
    }

    @SuppressWarnings("unchecked")
    void remove(@NonNull SingleDisposable<T> inner) {
        for (;;) {
            SingleDisposable<T>[] a = observers.get();
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
            SingleDisposable<T>[] b;
            if (n == 1) {
                b = EMPTY;
            } else {
                b = new SingleDisposable[n - 1];
                System.arraycopy(a, 0, b, 0, j);
                System.arraycopy(a, j + 1, b, j, n - j - 1);
            }

            if (observers.compareAndSet(a, b)) {
                return;
            }
        }
    }

    
    @Nullable
    public T getValue() {
        if (observers.get() == TERMINATED) {
            return value;
        }
        return null;
    }

    
    public boolean hasValue() {
        return observers.get() == TERMINATED && value != null;
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

    
    public boolean hasObservers() {
        return observers.get().length != 0;
    }

    
     int observerCount() {
        return observers.get().length;
    }

    static final class SingleDisposable<T>
    extends AtomicReference<SingleSubject<T>> implements Disposable {
        private static final long serialVersionUID = -7650903191002190468L;

        final SingleObserver<? super T> actual;

        SingleDisposable(SingleObserver<? super T> actual, SingleSubject<T> parent) {
            this.actual = actual;
            lazySet(parent);
        }

        @Override
        public void dispose() {
            SingleSubject<T> parent = getAndSet(null);
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
