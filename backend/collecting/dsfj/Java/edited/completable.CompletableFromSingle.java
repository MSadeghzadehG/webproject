

package io.reactivex.internal.operators.completable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;

public final class CompletableFromSingle<T> extends Completable {

    final SingleSource<T> single;

    public CompletableFromSingle(SingleSource<T> single) {
        this.single = single;
    }

    @Override
    protected void subscribeActual(final CompletableObserver s) {
        single.subscribe(new CompletableFromSingleObserver<T>(s));
    }

    static final class CompletableFromSingleObserver<T> implements SingleObserver<T> {
        final CompletableObserver co;

        CompletableFromSingleObserver(CompletableObserver co) {
            this.co = co;
        }

        @Override
        public void onError(Throwable e) {
            co.onError(e);
        }

        @Override
        public void onSubscribe(Disposable d) {
            co.onSubscribe(d);
        }

        @Override
        public void onSuccess(T value) {
            co.onComplete();
        }
    }
}
