

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.observers.ResumeSingleObserver;

public final class SingleDelayWithCompletable<T> extends Single<T> {

    final SingleSource<T> source;

    final CompletableSource other;

    public SingleDelayWithCompletable(SingleSource<T> source, CompletableSource other) {
        this.source = source;
        this.other = other;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> subscriber) {
        other.subscribe(new OtherObserver<T>(subscriber, source));
    }

    static final class OtherObserver<T>
    extends AtomicReference<Disposable>
    implements CompletableObserver, Disposable {


        private static final long serialVersionUID = -8565274649390031272L;

        final SingleObserver<? super T> actual;

        final SingleSource<T> source;

        OtherObserver(SingleObserver<? super T> actual, SingleSource<T> source) {
            this.actual = actual;
            this.source = source;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d)) {

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            source.subscribe(new ResumeSingleObserver<T>(this, actual));
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
