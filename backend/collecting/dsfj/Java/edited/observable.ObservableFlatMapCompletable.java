

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.annotations.Nullable;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.observers.BasicIntQueueDisposable;
import io.reactivex.internal.util.AtomicThrowable;
import io.reactivex.plugins.RxJavaPlugins;


public final class ObservableFlatMapCompletable<T> extends AbstractObservableWithUpstream<T, T> {

    final Function<? super T, ? extends CompletableSource> mapper;

    final boolean delayErrors;

    public ObservableFlatMapCompletable(ObservableSource<T> source,
            Function<? super T, ? extends CompletableSource> mapper, boolean delayErrors) {
        super(source);
        this.mapper = mapper;
        this.delayErrors = delayErrors;
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(new FlatMapCompletableMainObserver<T>(observer, mapper, delayErrors));
    }

    static final class FlatMapCompletableMainObserver<T> extends BasicIntQueueDisposable<T>
    implements Observer<T> {
        private static final long serialVersionUID = 8443155186132538303L;

        final Observer<? super T> actual;

        final AtomicThrowable errors;

        final Function<? super T, ? extends CompletableSource> mapper;

        final boolean delayErrors;

        final CompositeDisposable set;

        Disposable d;

        volatile boolean disposed;

        FlatMapCompletableMainObserver(Observer<? super T> observer, Function<? super T, ? extends CompletableSource> mapper, boolean delayErrors) {
            this.actual = observer;
            this.mapper = mapper;
            this.delayErrors = delayErrors;
            this.errors = new AtomicThrowable();
            this.set = new CompositeDisposable();
            this.lazySet(1);
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T value) {
            CompletableSource cs;

            try {
                cs = ObjectHelper.requireNonNull(mapper.apply(value), "The mapper returned a null CompletableSource");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                d.dispose();
                onError(ex);
                return;
            }

            getAndIncrement();

            InnerObserver inner = new InnerObserver();

            if (!disposed && set.add(inner)) {
                cs.subscribe(inner);
            }
        }

        @Override
        public void onError(Throwable e) {
            if (errors.addThrowable(e)) {
                if (delayErrors) {
                    if (decrementAndGet() == 0) {
                        Throwable ex = errors.terminate();
                        actual.onError(ex);
                    }
                } else {
                    dispose();
                    if (getAndSet(0) > 0) {
                        Throwable ex = errors.terminate();
                        actual.onError(ex);
                    }
                }
            } else {
                RxJavaPlugins.onError(e);
            }
        }

        @Override
        public void onComplete() {
            if (decrementAndGet() == 0) {
                Throwable ex = errors.terminate();
                if (ex != null) {
                    actual.onError(ex);
                } else {
                    actual.onComplete();
                }
            }
        }

        @Override
        public void dispose() {
            disposed = true;
            d.dispose();
            set.dispose();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Nullable
        @Override
        public T poll() throws Exception {
            return null;         }

        @Override
        public boolean isEmpty() {
            return true;         }

        @Override
        public void clear() {
                    }

        @Override
        public int requestFusion(int mode) {
            return mode & ASYNC;
        }

        void innerComplete(InnerObserver inner) {
            set.delete(inner);
            onComplete();
        }

        void innerError(InnerObserver inner, Throwable e) {
            set.delete(inner);
            onError(e);
        }

        final class InnerObserver extends AtomicReference<Disposable> implements CompletableObserver, Disposable {
            private static final long serialVersionUID = 8606673141535671828L;

            @Override
            public void onSubscribe(Disposable d) {
                DisposableHelper.setOnce(this, d);
            }

            @Override
            public void onComplete() {
                innerComplete(this);
            }

            @Override
            public void onError(Throwable e) {
                innerError(this, e);
            }

            @Override
            public void dispose() {
                DisposableHelper.dispose(this);
            }

            @Override
            public boolean isDisposed() {
                return DisposableHelper.isDisposed(get());
            }
        }
    }
}
