

package io.reactivex.internal.operators.mixed;

import java.util.concurrent.atomic.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;


public final class SingleFlatMapObservable<T, R> extends Observable<R> {

    final SingleSource<T> source;

    final Function<? super T, ? extends ObservableSource<? extends R>> mapper;

    public SingleFlatMapObservable(SingleSource<T> source,
            Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
        this.source = source;
        this.mapper = mapper;
    }

    @Override
    protected void subscribeActual(Observer<? super R> s) {
        FlatMapObserver<T, R> parent = new FlatMapObserver<T, R>(s, mapper);
        s.onSubscribe(parent);
        source.subscribe(parent);
    }

    static final class FlatMapObserver<T, R>
    extends AtomicReference<Disposable>
    implements Observer<R>, SingleObserver<T>, Disposable {

        private static final long serialVersionUID = -8948264376121066672L;

        final Observer<? super R> downstream;

        final Function<? super T, ? extends ObservableSource<? extends R>> mapper;

        FlatMapObserver(Observer<? super R> downstream, Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
            this.downstream = downstream;
            this.mapper = mapper;
        }

        @Override
        public void onNext(R t) {
            downstream.onNext(t);
        }

        @Override
        public void onError(Throwable t) {
            downstream.onError(t);
        }

        @Override
        public void onComplete() {
            downstream.onComplete();
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
            DisposableHelper.replace(this, d);
        }

        @Override
        public void onSuccess(T t) {
            ObservableSource<? extends R> o;

            try {
                o = ObjectHelper.requireNonNull(mapper.apply(t), "The mapper returned a null Publisher");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                downstream.onError(ex);
                return;
            }

            o.subscribe(this);
        }

    }
}
