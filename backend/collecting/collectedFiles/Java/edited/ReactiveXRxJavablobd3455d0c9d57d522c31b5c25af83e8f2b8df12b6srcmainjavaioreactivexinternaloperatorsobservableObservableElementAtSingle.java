

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.FuseToObservable;

import java.util.NoSuchElementException;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableElementAtSingle<T> extends Single<T> implements FuseToObservable<T> {
    final ObservableSource<T> source;
    final long index;
    final T defaultValue;

    public ObservableElementAtSingle(ObservableSource<T> source, long index, T defaultValue) {
        this.source = source;
        this.index = index;
        this.defaultValue = defaultValue;
    }

    @Override
    public void subscribeActual(SingleObserver<? super T> t) {
        source.subscribe(new ElementAtObserver<T>(t, index, defaultValue));
    }

    @Override
    public Observable<T> fuseToObservable() {
        return RxJavaPlugins.onAssembly(new ObservableElementAt<T>(source, index, defaultValue, true));
    }

    static final class ElementAtObserver<T> implements Observer<T>, Disposable {
        final SingleObserver<? super T> actual;
        final long index;
        final T defaultValue;

        Disposable s;

        long count;

        boolean done;

        ElementAtObserver(SingleObserver<? super T> actual, long index, T defaultValue) {
            this.actual = actual;
            this.index = index;
            this.defaultValue = defaultValue;
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
            long c = count;
            if (c == index) {
                done = true;
                s.dispose();
                actual.onSuccess(t);
                return;
            }
            count = c + 1;
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
            if (!done) {
                done = true;

                T v = defaultValue;

                if (v != null) {
                    actual.onSuccess(v);
                } else {
                    actual.onError(new NoSuchElementException());
                }
            }
        }
    }
}
