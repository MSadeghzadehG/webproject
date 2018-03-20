

package io.reactivex.internal.operators.single;

import java.util.concurrent.TimeUnit;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.SequentialDisposable;

public final class SingleDelay<T> extends Single<T> {

    final SingleSource<? extends T> source;
    final long time;
    final TimeUnit unit;
    final Scheduler scheduler;
    final boolean delayError;

    public SingleDelay(SingleSource<? extends T> source, long time, TimeUnit unit, Scheduler scheduler, boolean delayError) {
        this.source = source;
        this.time = time;
        this.unit = unit;
        this.scheduler = scheduler;
        this.delayError = delayError;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super T> s) {

        final SequentialDisposable sd = new SequentialDisposable();
        s.onSubscribe(sd);
        source.subscribe(new Delay(sd, s));
    }

    final class Delay implements SingleObserver<T> {
        private final SequentialDisposable sd;
        final SingleObserver<? super T> s;

        Delay(SequentialDisposable sd, SingleObserver<? super T> s) {
            this.sd = sd;
            this.s = s;
        }

        @Override
        public void onSubscribe(Disposable d) {
            sd.replace(d);
        }

        @Override
        public void onSuccess(final T value) {
            sd.replace(scheduler.scheduleDirect(new OnSuccess(value), time, unit));
        }

        @Override
        public void onError(final Throwable e) {
            sd.replace(scheduler.scheduleDirect(new OnError(e), delayError ? time : 0, unit));
        }

        final class OnSuccess implements Runnable {
            private final T value;

            OnSuccess(T value) {
                this.value = value;
            }

            @Override
            public void run() {
                s.onSuccess(value);
            }
        }

        final class OnError implements Runnable {
            private final Throwable e;

            OnError(Throwable e) {
                this.e = e;
            }

            @Override
            public void run() {
                s.onError(e);
            }
        }
    }
}
