

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicBoolean;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.plugins.RxJavaPlugins;


public final class MaybeAmb<T> extends Maybe<T> {
    private final MaybeSource<? extends T>[] sources;
    private final Iterable<? extends MaybeSource<? extends T>> sourcesIterable;

    public MaybeAmb(MaybeSource<? extends T>[] sources, Iterable<? extends MaybeSource<? extends T>> sourcesIterable) {
        this.sources = sources;
        this.sourcesIterable = sourcesIterable;
    }

    @Override
    @SuppressWarnings("unchecked")
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        MaybeSource<? extends T>[] sources = this.sources;
        int count = 0;
        if (sources == null) {
            sources = new MaybeSource[8];
            try {
                for (MaybeSource<? extends T> element : sourcesIterable) {
                    if (element == null) {
                        EmptyDisposable.error(new NullPointerException("One of the sources is null"), observer);
                        return;
                    }
                    if (count == sources.length) {
                        MaybeSource<? extends T>[] b = new MaybeSource[count + (count >> 2)];
                        System.arraycopy(sources, 0, b, 0, count);
                        sources = b;
                    }
                    sources[count++] = element;
                }
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                EmptyDisposable.error(e, observer);
                return;
            }
        } else {
            count = sources.length;
        }

        AmbMaybeObserver<T> parent = new AmbMaybeObserver<T>(observer);
        observer.onSubscribe(parent);

        for (int i = 0; i < count; i++) {
            MaybeSource<? extends T> s = sources[i];
            if (parent.isDisposed()) {
                return;
            }

            if (s == null) {
                parent.onError(new NullPointerException("One of the MaybeSources is null"));
                return;
            }

            s.subscribe(parent);
        }

        if (count == 0) {
            observer.onComplete();
        }

    }

    static final class AmbMaybeObserver<T>
    extends AtomicBoolean
    implements MaybeObserver<T>, Disposable {


        private static final long serialVersionUID = -7044685185359438206L;

        final MaybeObserver<? super T> actual;

        final CompositeDisposable set;

        AmbMaybeObserver(MaybeObserver<? super T> actual) {
            this.actual = actual;
            this.set = new CompositeDisposable();
        }

        @Override
        public void dispose() {
            if (compareAndSet(false, true)) {
                set.dispose();
            }
        }

        @Override
        public boolean isDisposed() {
            return get();
        }

        @Override
        public void onSubscribe(Disposable d) {
            set.add(d);
        }

        @Override
        public void onSuccess(T value) {
            if (compareAndSet(false, true)) {
                set.dispose();

                actual.onSuccess(value);
            }
        }

        @Override
        public void onError(Throwable e) {
            if (compareAndSet(false, true)) {
                set.dispose();

                actual.onError(e);
            } else {
                RxJavaPlugins.onError(e);
            }
        }

        @Override
        public void onComplete() {
            if (compareAndSet(false, true)) {
                set.dispose();

                actual.onComplete();
            }
        }

    }
}
