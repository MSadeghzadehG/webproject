

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class MaybeDelay<T> extends AbstractMaybeWithUpstream<T, T> {

    final long delay;

    final TimeUnit unit;

    final Scheduler scheduler;

    public MaybeDelay(MaybeSource<T> source, long delay, TimeUnit unit, Scheduler scheduler) {
        super(source);
        this.delay = delay;
        this.unit = unit;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(new DelayMaybeObserver<T>(observer, delay, unit, scheduler));
    }

    static final class DelayMaybeObserver<T>
    extends AtomicReference<Disposable>
    implements MaybeObserver<T>, Disposable, Runnable {

        private static final long serialVersionUID = 5566860102500855068L;

        final MaybeObserver<? super T> actual;

        final long delay;

        final TimeUnit unit;

        final Scheduler scheduler;

        T value;

        Throwable error;

        DelayMaybeObserver(MaybeObserver<? super T> actual, long delay, TimeUnit unit, Scheduler scheduler) {
            this.actual = actual;
            this.delay = delay;
            this.unit = unit;
            this.scheduler = scheduler;
        }

        @Override
        public void run() {
            Throwable ex = error;
            if (ex != null) {
                actual.onError(ex);
            } else {
                T v = value;
                if (v != null) {
                    actual.onSuccess(v);
                } else {
                    actual.onComplete();
                }
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

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.setOnce(this, d)) {
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            this.value = value;
            schedule();
        }

        @Override
        public void onError(Throwable e) {
            this.error = e;
            schedule();
        }

        @Override
        public void onComplete() {
            schedule();
        }

        void schedule() {
            DisposableHelper.replace(this, scheduler.scheduleDirect(this, delay, unit));
        }
    }
}
