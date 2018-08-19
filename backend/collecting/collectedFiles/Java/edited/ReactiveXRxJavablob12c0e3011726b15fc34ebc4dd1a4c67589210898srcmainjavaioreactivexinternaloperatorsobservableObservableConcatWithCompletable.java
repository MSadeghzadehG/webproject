

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class ObservableConcatWithCompletable<T> extends AbstractObservableWithUpstream<T, T> {

    final CompletableSource other;

    public ObservableConcatWithCompletable(Observable<T> source, CompletableSource other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(new ConcatWithObserver<T>(observer, other));
    }

    static final class ConcatWithObserver<T>
    extends AtomicReference<Disposable>
    implements Observer<T>, CompletableObserver, Disposable {

        private static final long serialVersionUID = -1953724749712440952L;

        final Observer<? super T> actual;

        CompletableSource other;

        boolean inCompletable;

        ConcatWithObserver(Observer<? super T> actual, CompletableSource other) {
            this.actual = actual;
            this.other = other;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d) && !inCompletable) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            if (inCompletable) {
                actual.onComplete();
            } else {
                inCompletable = true;
                DisposableHelper.replace(this, null);
                CompletableSource cs = other;
                other = null;
                cs.subscribe(this);
            }
        }

        @Override
        public void dispose() {
            DisposableHelper.dispose(this);
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }
    }
}
