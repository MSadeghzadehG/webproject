

package io.reactivex.internal.operators.observable;

import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.annotations.Nullable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.fuseable.QueueDisposable;
import io.reactivex.plugins.RxJavaPlugins;


public final class ObservableScalarXMap {

    
    private ObservableScalarXMap() {
        throw new IllegalStateException("No instances!");
    }

    
    @SuppressWarnings("unchecked")
    public static <T, R> boolean tryScalarXMapSubscribe(ObservableSource<T> source,
            Observer<? super R> observer,
            Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
        if (source instanceof Callable) {
            T t;

            try {
                t = ((Callable<T>)source).call();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                EmptyDisposable.error(ex, observer);
                return true;
            }

            if (t == null) {
                EmptyDisposable.complete(observer);
                return true;
            }

            ObservableSource<? extends R> r;

            try {
                r = ObjectHelper.requireNonNull(mapper.apply(t), "The mapper returned a null ObservableSource");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                EmptyDisposable.error(ex, observer);
                return true;
            }

            if (r instanceof Callable) {
                R u;

                try {
                    u = ((Callable<R>)r).call();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    EmptyDisposable.error(ex, observer);
                    return true;
                }

                if (u == null) {
                    EmptyDisposable.complete(observer);
                    return true;
                }
                ScalarDisposable<R> sd = new ScalarDisposable<R>(observer, u);
                observer.onSubscribe(sd);
                sd.run();
            } else {
                r.subscribe(observer);
            }

            return true;
        }
        return false;
    }

    
    public static <T, U> Observable<U> scalarXMap(T value,
            Function<? super T, ? extends ObservableSource<? extends U>> mapper) {
        return RxJavaPlugins.onAssembly(new ScalarXMapObservable<T, U>(value, mapper));
    }

    
    static final class ScalarXMapObservable<T, R> extends Observable<R> {

        final T value;

        final Function<? super T, ? extends ObservableSource<? extends R>> mapper;

        ScalarXMapObservable(T value,
                Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
            this.value = value;
            this.mapper = mapper;
        }

        @SuppressWarnings("unchecked")
        @Override
        public void subscribeActual(Observer<? super R> s) {
            ObservableSource<? extends R> other;
            try {
                other = ObjectHelper.requireNonNull(mapper.apply(value), "The mapper returned a null ObservableSource");
            } catch (Throwable e) {
                EmptyDisposable.error(e, s);
                return;
            }
            if (other instanceof Callable) {
                R u;

                try {
                    u = ((Callable<R>)other).call();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    EmptyDisposable.error(ex, s);
                    return;
                }

                if (u == null) {
                    EmptyDisposable.complete(s);
                    return;
                }
                ScalarDisposable<R> sd = new ScalarDisposable<R>(s, u);
                s.onSubscribe(sd);
                sd.run();
            } else {
                other.subscribe(s);
            }
        }
    }

    
    public static final class ScalarDisposable<T>
    extends AtomicInteger
    implements QueueDisposable<T>, Runnable {

        private static final long serialVersionUID = 3880992722410194083L;

        final Observer<? super T> observer;

        final T value;

        static final int START = 0;
        static final int FUSED = 1;
        static final int ON_NEXT = 2;
        static final int ON_COMPLETE = 3;

        public ScalarDisposable(Observer<? super T> observer, T value) {
            this.observer = observer;
            this.value = value;
        }

        @Override
        public boolean offer(T value) {
            throw new UnsupportedOperationException("Should not be called!");
        }

        @Override
        public boolean offer(T v1, T v2) {
            throw new UnsupportedOperationException("Should not be called!");
        }

        @Nullable
        @Override
        public T poll() throws Exception {
            if (get() == FUSED) {
                lazySet(ON_COMPLETE);
                return value;
            }
            return null;
        }

        @Override
        public boolean isEmpty() {
            return get() != FUSED;
        }

        @Override
        public void clear() {
            lazySet(ON_COMPLETE);
        }

        @Override
        public void dispose() {
            set(ON_COMPLETE);
        }

        @Override
        public boolean isDisposed() {
            return get() == ON_COMPLETE;
        }

        @Override
        public int requestFusion(int mode) {
            if ((mode & SYNC) != 0) {
                lazySet(FUSED);
                return SYNC;
            }
            return NONE;
        }

        @Override
        public void run() {
            if (get() == START && compareAndSet(START, ON_NEXT)) {
                observer.onNext(value);
                if (get() == ON_NEXT) {
                    lazySet(ON_COMPLETE);
                    observer.onComplete();
                }
            }
        }
    }
}
