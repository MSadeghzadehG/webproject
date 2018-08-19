

package io.reactivex.internal.operators.completable;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class CompletableTimer extends Completable {

    final long delay;
    final TimeUnit unit;
    final Scheduler scheduler;

    public CompletableTimer(long delay, TimeUnit unit, Scheduler scheduler) {
        this.delay = delay;
        this.unit = unit;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(final CompletableObserver s) {
        TimerDisposable parent = new TimerDisposable(s);
        s.onSubscribe(parent);
        parent.setFuture(scheduler.scheduleDirect(parent, delay, unit));
    }

    static final class TimerDisposable extends AtomicReference<Disposable> implements Disposable, Runnable {

        private static final long serialVersionUID = 3167244060586201109L;
        final CompletableObserver actual;

        TimerDisposable(final CompletableObserver actual) {
            this.actual = actual;
        }

        @Override
        public void run() {
            actual.onComplete();
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
