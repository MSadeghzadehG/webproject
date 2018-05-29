

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;

public final class SingleObserveOn<T> extends Single<T> {

    final SingleSource<T> source;

    final Scheduler scheduler;

    public SingleObserveOn(SingleSource<T> source, Scheduler scheduler) {
        this.source = source;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super T> s) {
        source.subscribe(new ObserveOnSingleObserver<T>(s, scheduler));
    }

    static final class ObserveOnSingleObserver<T> extends AtomicReference<Disposable>
    implements SingleObserver<T>, Disposable, Runnable {
        private static final long serialVersionUID = 3528003840217436037L;

        final SingleObserver<? super T> actual;

        final Scheduler scheduler;

        T value;
        Throwable error;

        ObserveOnSingleObserver(SingleObserver<? super T> actual, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d)) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            this.value = value;
            Disposable d = scheduler.scheduleDirect(this);
            DisposableHelper.replace(this, d);
        }

        @Override
        public void onError(Throwable e) {
            this.error = e;
            Disposable d = scheduler.scheduleDirect(this);
            DisposableHelper.replace(this, d);
        }

        @Override
        public void run() {
            Throwable ex = error;
            if (ex != null) {
                actual.onError(ex);
            } else {
                actual.onSuccess(value);
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
