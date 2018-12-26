

package io.reactivex.internal.operators.completable;

import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.*;
import io.reactivex.functions.*;
import io.reactivex.internal.disposables.*;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class CompletableUsing<R> extends Completable {

    final Callable<R> resourceSupplier;
    final Function<? super R, ? extends CompletableSource> completableFunction;
    final Consumer<? super R> disposer;
    final boolean eager;

    public CompletableUsing(Callable<R> resourceSupplier,
                            Function<? super R, ? extends CompletableSource> completableFunction, Consumer<? super R> disposer,
                            boolean eager) {
        this.resourceSupplier = resourceSupplier;
        this.completableFunction = completableFunction;
        this.disposer = disposer;
        this.eager = eager;
    }


    @Override
    protected void subscribeActual(CompletableObserver observer) {
        R resource;

        try {
            resource = resourceSupplier.call();
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            EmptyDisposable.error(ex, observer);
            return;
        }

        CompletableSource source;

        try {
            source = ObjectHelper.requireNonNull(completableFunction.apply(resource), "The completableFunction returned a null CompletableSource");
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            if (eager) {
                try {
                    disposer.accept(resource);
                } catch (Throwable exc) {
                    Exceptions.throwIfFatal(exc);
                    EmptyDisposable.error(new CompositeException(ex, exc), observer);
                    return;
                }
            }

            EmptyDisposable.error(ex, observer);

            if (!eager) {
                try {
                    disposer.accept(resource);
                } catch (Throwable exc) {
                    Exceptions.throwIfFatal(exc);
                    RxJavaPlugins.onError(exc);
                }
            }
            return;
        }

        source.subscribe(new UsingObserver<R>(observer, resource, disposer, eager));
    }

    static final class UsingObserver<R>
    extends AtomicReference<Object>
    implements CompletableObserver, Disposable {


        private static final long serialVersionUID = -674404550052917487L;

        final CompletableObserver actual;

        final Consumer<? super R> disposer;

        final boolean eager;

        Disposable d;

        UsingObserver(CompletableObserver actual, R resource, Consumer<? super R> disposer, boolean eager) {
            super(resource);
            this.actual = actual;
            this.disposer = disposer;
            this.eager = eager;
        }

        @Override
        public void dispose() {
            d.dispose();
            d = DisposableHelper.DISPOSED;
            disposeResourceAfter();
        }

        @SuppressWarnings("unchecked")
        void disposeResourceAfter() {
            Object resource = getAndSet(this);
            if (resource != this) {
                try {
                    disposer.accept((R)resource);
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
            }
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @SuppressWarnings("unchecked")
        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            if (eager) {
                Object resource = getAndSet(this);
                if (resource != this) {
                    try {
                        disposer.accept((R)resource);
                    } catch (Throwable ex) {
                        Exceptions.throwIfFatal(ex);
                        e = new CompositeException(e, ex);
                    }
                } else {
                    return;
                }
            }

            actual.onError(e);

            if (!eager) {
                disposeResourceAfter();
            }
        }

        @SuppressWarnings("unchecked")
        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            if (eager) {
                Object resource = getAndSet(this);
                if (resource != this) {
                    try {
                        disposer.accept((R)resource);
                    } catch (Throwable ex) {
                        Exceptions.throwIfFatal(ex);
                        actual.onError(ex);
                        return;
                    }
                } else {
                    return;
                }
            }

            actual.onComplete();

            if (!eager) {
                disposeResourceAfter();
            }
        }
    }

}
