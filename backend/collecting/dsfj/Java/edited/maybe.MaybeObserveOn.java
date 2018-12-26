

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class MaybeObserveOn<T> extends AbstractMaybeWithUpstream<T, T> {

    final Scheduler scheduler;

    public MaybeObserveOn(MaybeSource<T> source, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(new ObserveOnMaybeObserver<T>(observer, scheduler));
    }

    static final class ObserveOnMaybeObserver<T>
    extends AtomicReference<Disposable>
    implements MaybeObserver<T>, Disposable, Runnable {


        private static final long serialVersionUID = 8571289934935992137L;

        final MaybeObserver<? super T> actual;

        final Scheduler scheduler;

        T value;
        Throwable error;

        ObserveOnMaybeObserver(MaybeObserver<? super T> actual, Scheduler scheduler) {
            this.actual = actual;
            this.scheduler = scheduler;
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
            DisposableHelper.replace(this, scheduler.scheduleDirect(this));
        }

        @Override
        public void onError(Throwable e) {
            this.error = e;
            DisposableHelper.replace(this, scheduler.scheduleDirect(this));
        }

        @Override
        public void onComplete() {
            DisposableHelper.replace(this, scheduler.scheduleDirect(this));
        }

        @Override
        public void run() {
            Throwable ex = error;
            if (ex != null) {
                error = null;
                actual.onError(ex);
            } else {
                T v = value;
                if (v != null) {
                    value = null;
                    actual.onSuccess(v);
                } else {
                    actual.onComplete();
                }
            }
        }
    }
}
