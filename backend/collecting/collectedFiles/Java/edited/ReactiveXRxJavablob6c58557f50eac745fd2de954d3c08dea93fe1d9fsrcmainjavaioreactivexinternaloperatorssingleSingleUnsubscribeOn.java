

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class SingleUnsubscribeOn<T> extends Single<T> {

    final SingleSource<T> source;

    final Scheduler scheduler;

    public SingleUnsubscribeOn(SingleSource<T> source, Scheduler scheduler) {
        this.source = source;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super T> observer) {
        source.subscribe(new UnsubscribeOnSingleObserver<T>(observer, scheduler));
    }

    static final class UnsubscribeOnSingleObserver<T> extends AtomicReference<Disposable>
    implements SingleObserver<T>, Disposable, Runnable {

        private static final long serialVersionUID = 3256698449646456986L;

        final SingleObserver<? super T> actual;

        final Scheduler scheduler;

        Disposable ds;

        UnsubscribeOnSingleObserver(SingleObserver<? super T> actual, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
        }

        @Override
        public void dispose() {
            Disposable d = getAndSet(DisposableHelper.DISPOSED);
            if (d != DisposableHelper.DISPOSED) {
                this.ds = d;
                scheduler.scheduleDirect(this);
            }
        }

        @Override
        public void run() {
            ds.dispose();
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d)) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            actual.onSuccess(value);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }
    }
}
