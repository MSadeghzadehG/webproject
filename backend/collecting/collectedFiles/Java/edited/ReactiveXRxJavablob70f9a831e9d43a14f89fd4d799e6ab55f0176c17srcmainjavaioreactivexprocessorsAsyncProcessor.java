
package io.reactivex.processors;

import java.util.Arrays;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.annotations.*;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.DeferredScalarSubscription;
import io.reactivex.plugins.RxJavaPlugins;


public final class AsyncProcessor<T> extends FlowableProcessor<T> {

    @SuppressWarnings("rawtypes")
    static final AsyncSubscription[] EMPTY = new AsyncSubscription[0];

    @SuppressWarnings("rawtypes")
    static final AsyncSubscription[] TERMINATED = new AsyncSubscription[0];

    final AtomicReference<AsyncSubscription<T>[]> subscribers;

    
    Throwable error;

    
    T value;

    
    @CheckReturnValue
    @NonNull
    public static <T> AsyncProcessor<T> create() {
        return new AsyncProcessor<T>();
    }

    
    @SuppressWarnings("unchecked")
    AsyncProcessor() {
        this.subscribers = new AtomicReference<AsyncSubscription<T>[]>(EMPTY);
    }

    @Override
    public void onSubscribe(Subscription s) {
        if (subscribers.get() == TERMINATED) {
            s.cancel();
            return;
        }
                s.request(Long.MAX_VALUE);
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
        for (AsyncSubscription<T> as : subscribers.getAndSet(TERMINATED)) {
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
        AsyncSubscription<T>[] array = subscribers.getAndSet(TERMINATED);
        if (v == null) {
            for (AsyncSubscription<T> as : array) {
                as.onComplete();
            }
        } else {
            for (AsyncSubscription<T> as : array) {
                as.complete(v);
            }
        }
    }

    @Override
    public boolean hasSubscribers() {
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
    @Nullable
    public Throwable getThrowable() {
        return subscribers.get() == TERMINATED ? error : null;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        AsyncSubscription<T> as = new AsyncSubscription<T>(s, this);
        s.onSubscribe(as);
        if (add(as)) {
            if (as.isCancelled()) {
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

    
    boolean add(AsyncSubscription<T> ps) {
        for (;;) {
            AsyncSubscription<T>[] a = subscribers.get();
            if (a == TERMINATED) {
                return false;
            }

            int n = a.length;
            @SuppressWarnings("unchecked")
            AsyncSubscription<T>[] b = new AsyncSubscription[n + 1];
            System.arraycopy(a, 0, b, 0, n);
            b[n] = ps;

            if (subscribers.compareAndSet(a, b)) {
                return true;
            }
        }
    }

    
    @SuppressWarnings("unchecked")
    void remove(AsyncSubscription<T> ps) {
        for (;;) {
            AsyncSubscription<T>[] a = subscribers.get();
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

            AsyncSubscription<T>[] b;

            if (n == 1) {
                b = EMPTY;
            } else {
                b = new AsyncSubscription[n - 1];
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

    
    @Deprecated
    public Object[] getValues() {
        T v = getValue();
        return v != null ? new Object[] { v } : new Object[0];
    }

    
    @Deprecated
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

    static final class AsyncSubscription<T> extends DeferredScalarSubscription<T> {
        private static final long serialVersionUID = 5629876084736248016L;

        final AsyncProcessor<T> parent;

        AsyncSubscription(Subscriber<? super T> actual, AsyncProcessor<T> parent) {
            super(actual);
            this.parent = parent;
        }

        @Override
        public void cancel() {
            if (super.tryCancel()) {
                parent.remove(this);
            }
        }

        void onComplete() {
            if (!isCancelled()) {
                actual.onComplete();
            }
        }

        void onError(Throwable t) {
            if (isCancelled()) {
                RxJavaPlugins.onError(t);
            } else {
                actual.onError(t);
            }
        }
    }
}
