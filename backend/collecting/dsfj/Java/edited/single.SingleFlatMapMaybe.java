

package io.reactivex.internal.operators.single;

import io.reactivex.Maybe;
import io.reactivex.MaybeObserver;
import io.reactivex.MaybeSource;
import io.reactivex.SingleObserver;
import io.reactivex.SingleSource;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;
import java.util.concurrent.atomic.AtomicReference;

public final class SingleFlatMapMaybe<T, R> extends Maybe<R> {

    final SingleSource<? extends T> source;

    final Function<? super T, ? extends MaybeSource<? extends R>> mapper;

    public SingleFlatMapMaybe(SingleSource<? extends T> source, Function<? super T, ? extends MaybeSource<? extends R>> mapper) {
        this.mapper = mapper;
        this.source = source;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super R> actual) {
        source.subscribe(new FlatMapSingleObserver<T, R>(actual, mapper));
    }

    static final class FlatMapSingleObserver<T, R>
    extends AtomicReference<Disposable>
    implements SingleObserver<T>, Disposable {

        private static final long serialVersionUID = -5843758257109742742L;

        final MaybeObserver<? super R> actual;

        final Function<? super T, ? extends MaybeSource<? extends R>> mapper;

        FlatMapSingleObserver(MaybeObserver<? super R> actual, Function<? super T, ? extends MaybeSource<? extends R>> mapper) {
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
            MaybeSource<? extends R> ms;

            try {
                ms = ObjectHelper.requireNonNull(mapper.apply(value), "The mapper returned a null MaybeSource");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                onError(ex);
                return;
            }

            if (!isDisposed()) {
                ms.subscribe(new FlatMapMaybeObserver<R>(this, actual));
            }
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }
    }

    static final class FlatMapMaybeObserver<R> implements MaybeObserver<R> {

        final AtomicReference<Disposable> parent;

        final MaybeObserver<? super R> actual;

        FlatMapMaybeObserver(AtomicReference<Disposable> parent, MaybeObserver<? super R> actual) {
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

        @Override
        public void onComplete() {
            actual.onComplete();
        }
    }
}
