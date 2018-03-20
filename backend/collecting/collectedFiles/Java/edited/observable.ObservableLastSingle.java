

package io.reactivex.internal.operators.observable;

import java.util.NoSuchElementException;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class ObservableLastSingle<T> extends Single<T> {

    final ObservableSource<T> source;

    final T defaultItem;

    public ObservableLastSingle(ObservableSource<T> source, T defaultItem) {
        this.source = source;
        this.defaultItem = defaultItem;
    }

    
    @Override
    protected void subscribeActual(SingleObserver<? super T> observer) {
        source.subscribe(new LastObserver<T>(observer, defaultItem));
    }

    static final class LastObserver<T> implements Observer<T>, Disposable {

        final SingleObserver<? super T> actual;

        final T defaultItem;

        Disposable s;

        T item;

        LastObserver(SingleObserver<? super T> actual, T defaultItem) {
            this.actual = actual;
            this.defaultItem = defaultItem;
        }

        @Override
        public void dispose() {
            s.dispose();
            s = DisposableHelper.DISPOSED;
        }

        @Override
        public boolean isDisposed() {
            return s == DisposableHelper.DISPOSED;
        }

        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.s, s)) {
                this.s = s;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            item = t;
        }

        @Override
        public void onError(Throwable t) {
            s = DisposableHelper.DISPOSED;
            item = null;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            s = DisposableHelper.DISPOSED;
            T v = item;
            if (v != null) {
                item = null;
                actual.onSuccess(v);
            } else {
                v = defaultItem;
                if (v != null) {
                    actual.onSuccess(v);
                } else {
                    actual.onError(new NoSuchElementException());
                }
            }
        }
    }
}
