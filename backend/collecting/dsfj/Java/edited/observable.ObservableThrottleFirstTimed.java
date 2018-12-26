

package io.reactivex.internal.operators.observable;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.Scheduler.Worker;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;
import io.reactivex.observers.SerializedObserver;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableThrottleFirstTimed<T> extends AbstractObservableWithUpstream<T, T> {
    final long timeout;
    final TimeUnit unit;
    final Scheduler scheduler;

    public ObservableThrottleFirstTimed(ObservableSource<T> source,
            long timeout, TimeUnit unit, Scheduler scheduler) {
        super(source);
        this.timeout = timeout;
        this.unit = unit;
        this.scheduler = scheduler;
    }

    @Override
    public void subscribeActual(Observer<? super T> t) {
        source.subscribe(new DebounceTimedObserver<T>(
                new SerializedObserver<T>(t),
                timeout, unit, scheduler.createWorker()));
    }

    static final class DebounceTimedObserver<T>
    extends AtomicReference<Disposable>
    implements Observer<T>, Disposable, Runnable {
        private static final long serialVersionUID = 786994795061867455L;

        final Observer<? super T> actual;
        final long timeout;
        final TimeUnit unit;
        final Scheduler.Worker worker;

        Disposable s;

        volatile boolean gate;

        boolean done;

        DebounceTimedObserver(Observer<? super T> actual, long timeout, TimeUnit unit, Worker worker) {
            this.actual = actual;
            this.timeout = timeout;
            this.unit = unit;
            this.worker = worker;
        }

        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.s, s)) {
                this.s = s;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            if (!gate && !done) {
                gate = true;

                actual.onNext(t);

                Disposable d = get();
                if (d != null) {
                    d.dispose();
                }
                DisposableHelper.replace(this, worker.schedule(this, timeout, unit));
            }


        }

        @Override
        public void run() {
            gate = false;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
            } else {
                done = true;
                actual.onError(t);
                worker.dispose();
            }
        }

        @Override
        public void onComplete() {
            if (!done) {
                done = true;
                actual.onComplete();
                worker.dispose();
            }
        }

        @Override
        public void dispose() {
            s.dispose();
            worker.dispose();
        }

        @Override
        public boolean isDisposed() {
            return worker.isDisposed();
        }
    }
}
