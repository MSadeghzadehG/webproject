

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicBoolean;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableUnsubscribeOn<T> extends AbstractObservableWithUpstream<T, T> {
    final Scheduler scheduler;
    public ObservableUnsubscribeOn(ObservableSource<T> source, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
    }

    @Override
    public void subscribeActual(Observer<? super T> t) {
        source.subscribe(new UnsubscribeObserver<T>(t, scheduler));
    }

    static final class UnsubscribeObserver<T> extends AtomicBoolean implements Observer<T>, Disposable {

        private static final long serialVersionUID = 1015244841293359600L;

        final Observer<? super T> actual;
        final Scheduler scheduler;

        Disposable s;

        UnsubscribeObserver(Observer<? super T> actual, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
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
            if (!get()) {
                actual.onNext(t);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (get()) {
                RxJavaPlugins.onError(t);
                return;
            }
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (!get()) {
                actual.onComplete();
            }
        }

        @Override
        public void dispose() {
            if (compareAndSet(false, true)) {
                scheduler.scheduleDirect(new DisposeTask());
            }
        }

        @Override
        public boolean isDisposed() {
            return get();
        }

        final class DisposeTask implements Runnable {
            @Override
            public void run() {
                s.dispose();
            }
        }
    }
}
