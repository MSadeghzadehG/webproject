

package io.reactivex.internal.operators.observable;

import java.util.concurrent.TimeUnit;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.schedulers.Timed;

public final class ObservableTimeInterval<T> extends AbstractObservableWithUpstream<T, Timed<T>> {
    final Scheduler scheduler;
    final TimeUnit unit;

    public ObservableTimeInterval(ObservableSource<T> source, TimeUnit unit, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
        this.unit = unit;
    }

    @Override
    public void subscribeActual(Observer<? super Timed<T>> t) {
        source.subscribe(new TimeIntervalObserver<T>(t, unit, scheduler));
    }

    static final class TimeIntervalObserver<T> implements Observer<T>, Disposable {
        final Observer<? super Timed<T>> actual;
        final TimeUnit unit;
        final Scheduler scheduler;

        long lastTime;

        Disposable s;

        TimeIntervalObserver(Observer<? super Timed<T>> actual, TimeUnit unit, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
            this.unit = unit;
        }

        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.s, s)) {
                this.s = s;
                lastTime = scheduler.now(unit);
                actual.onSubscribe(this);
            }
        }

        @Override
        public void dispose() {
            s.dispose();
        }

        @Override
        public boolean isDisposed() {
            return s.isDisposed();
        }


        @Override
        public void onNext(T t) {
            long now = scheduler.now(unit);
            long last = lastTime;
            lastTime = now;
            long delta = now - last;
            actual.onNext(new Timed<T>(t, delta, unit));
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }
    }
}
