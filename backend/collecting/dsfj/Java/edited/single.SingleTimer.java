

package io.reactivex.internal.operators.single;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class SingleTimer extends Single<Long> {

    final long delay;
    final TimeUnit unit;
    final Scheduler scheduler;

    public SingleTimer(long delay, TimeUnit unit, Scheduler scheduler) {
        this.delay = delay;
        this.unit = unit;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super Long> s) {
        TimerDisposable parent = new TimerDisposable(s);
        s.onSubscribe(parent);
        parent.setFuture(scheduler.scheduleDirect(parent, delay, unit));
    }

    static final class TimerDisposable extends AtomicReference<Disposable> implements Disposable, Runnable {

        private static final long serialVersionUID = 8465401857522493082L;
        final SingleObserver<? super Long> actual;

        TimerDisposable(final SingleObserver<? super Long> actual) {
            this.actual = actual;
        }

        @Override
        public void run() {
            actual.onSuccess(0L);
        }

        @Override
        public void dispose() {
            DisposableHelper.dispose(this);
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }

        void setFuture(Disposable d) {
            DisposableHelper.replace(this, d);
        }
    }
}
