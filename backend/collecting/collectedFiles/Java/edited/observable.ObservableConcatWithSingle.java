

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class ObservableConcatWithSingle<T> extends AbstractObservableWithUpstream<T, T> {

    final SingleSource<? extends T> other;

    public ObservableConcatWithSingle(Observable<T> source, SingleSource<? extends T> other) {
        super(source);
        this.other = other;
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(new ConcatWithObserver<T>(observer, other));
    }

    static final class ConcatWithObserver<T>
    extends AtomicReference<Disposable>
    implements Observer<T>, SingleObserver<T>, Disposable {

        private static final long serialVersionUID = -1953724749712440952L;

        final Observer<? super T> actual;

        SingleSource<? extends T> other;

        boolean inSingle;

        ConcatWithObserver(Observer<? super T> actual, SingleSource<? extends T> other) {
            this.actual = actual;
            this.other = other;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d) && !inSingle) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public void onSuccess(T t) {
            actual.onNext(t);
            actual.onComplete();
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            inSingle = true;
            DisposableHelper.replace(this, null);
            SingleSource<? extends T> ss = other;
            other = null;
            ss.subscribe(this);
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
