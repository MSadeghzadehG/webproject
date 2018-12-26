

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableSingleMaybe<T> extends Maybe<T> {

    final ObservableSource<T> source;

    public ObservableSingleMaybe(ObservableSource<T> source) {
        this.source = source;
    }
    @Override
    public void subscribeActual(MaybeObserver<? super T> t) {
        source.subscribe(new SingleElementObserver<T>(t));
    }

    static final class SingleElementObserver<T> implements Observer<T>, Disposable {
        final MaybeObserver<? super T> actual;

        Disposable s;

        T value;

        boolean done;

        SingleElementObserver(MaybeObserver<? super T> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.s, s)) {
                this.s = s;
                actual.onSubscribe(this);
            }
        }


        @Override
        public void dispose() {
            s.dispose();
        }

        @Override
        public boolean isDisposed() {
            return s.isDisposed();
        }


        @Override
        public void onNext(T t) {
            if (done) {
                return;
            }
            if (value != null) {
                done = true;
                s.dispose();
                actual.onError(new IllegalArgumentException("Sequence contains more than one element!"));
                return;
            }
            value = t;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            T v = value;
            value = null;
            if (v == null) {
                actual.onComplete();
            } else {
                actual.onSuccess(v);
            }
        }
    }
}
