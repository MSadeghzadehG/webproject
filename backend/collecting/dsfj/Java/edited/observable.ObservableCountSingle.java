

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.FuseToObservable;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableCountSingle<T> extends Single<Long> implements FuseToObservable<Long> {
    final ObservableSource<T> source;
    public ObservableCountSingle(ObservableSource<T> source) {
        this.source = source;
    }

    @Override
    public void subscribeActual(SingleObserver<? super Long> t) {
        source.subscribe(new CountObserver(t));
    }

    @Override
    public Observable<Long> fuseToObservable() {
        return RxJavaPlugins.onAssembly(new ObservableCount<T>(source));
    }

    static final class CountObserver implements Observer<Object>, Disposable {
        final SingleObserver<? super Long> actual;

        Disposable d;

        long count;

        CountObserver(SingleObserver<? super Long> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;
                actual.onSubscribe(this);
            }
        }


        @Override
        public void dispose() {
            d.dispose();
            d = DisposableHelper.DISPOSED;
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void onNext(Object t) {
            count++;
        }

        @Override
        public void onError(Throwable t) {
            d = DisposableHelper.DISPOSED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            actual.onSuccess(count);
        }
    }
}
