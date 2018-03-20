

package io.reactivex.internal.operators.single;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.atomic.AtomicReference;

public final class SingleFlatMap<T, R> extends Single<R> {
    final SingleSource<? extends T> source;

    final Function<? super T, ? extends SingleSource<? extends R>> mapper;

    public SingleFlatMap(SingleSource<? extends T> source, Function<? super T, ? extends SingleSource<? extends R>> mapper) {
        this.mapper = mapper;
        this.source = source;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super R> actual) {
        source.subscribe(new SingleFlatMapCallback<T, R>(actual, mapper));
    }

    static final class SingleFlatMapCallback<T, R>
    extends AtomicReference<Disposable>
    implements SingleObserver<T>, Disposable {
        private static final long serialVersionUID = 3258103020495908596L;

        final SingleObserver<? super R> actual;

        final Function<? super T, ? extends SingleSource<? extends R>> mapper;

        SingleFlatMapCallback(SingleObserver<? super R> actual,
                Function<? super T, ? extends SingleSource<? extends R>> mapper) {
            this.actual = actual;
            this.mapper = mapper;
        }

        @Override
        public void dispose() {
            DisposableHelper.dispose(this);
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d)) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            SingleSource<? extends R> o;

            try {
                o = ObjectHelper.requireNonNull(mapper.apply(value), "The single returned by the mapper is null");
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                actual.onError(e);
                return;
            }

            if (!isDisposed()) {
                o.subscribe(new FlatMapSingleObserver<R>(this, actual));
            }
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        static final class FlatMapSingleObserver<R> implements SingleObserver<R> {

            final AtomicReference<Disposable> parent;

            final SingleObserver<? super R> actual;

            FlatMapSingleObserver(AtomicReference<Disposable> parent, SingleObserver<? super R> actual) {
                this.parent = parent;
                this.actual = actual;
            }

            @Override
            public void onSubscribe(final Disposable d) {
                DisposableHelper.replace(parent, d);
            }

            @Override
            public void onSuccess(final R value) {
                actual.onSuccess(value);
            }

            @Override
            public void onError(final Throwable e) {
                actual.onError(e);
            }
        }
    }
}
