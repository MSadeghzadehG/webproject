

package io.reactivex.internal.operators.completable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;

public final class CompletableFromObservable<T> extends Completable {

    final ObservableSource<T> observable;

    public CompletableFromObservable(ObservableSource<T> observable) {
        this.observable = observable;
    }

    @Override
    protected void subscribeActual(final CompletableObserver s) {
        observable.subscribe(new CompletableFromObservableObserver<T>(s));
    }

    static final class CompletableFromObservableObserver<T> implements Observer<T> {
        final CompletableObserver co;

        CompletableFromObservableObserver(CompletableObserver co) {
            this.co = co;
        }

        @Override
        public void onSubscribe(Disposable d) {
            co.onSubscribe(d);
        }

        @Override
        public void onNext(T value) {
                    }

        @Override
        public void onError(Throwable e) {
            co.onError(e);
        }

        @Override
        public void onComplete() {
            co.onComplete();
        }
    }
}
