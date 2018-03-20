

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;


public final class MaybeUnsubscribeOn<T> extends AbstractMaybeWithUpstream<T, T> {

    final Scheduler scheduler;

    public MaybeUnsubscribeOn(MaybeSource<T> source, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(new UnsubscribeOnMaybeObserver<T>(observer, scheduler));
    }

    static final class UnsubscribeOnMaybeObserver<T> extends AtomicReference<Disposable>
    implements MaybeObserver<T>, Disposable, Runnable {

        private static final long serialVersionUID = 3256698449646456986L;

        final MaybeObserver<? super T> actual;

        final Scheduler scheduler;

        Disposable ds;

        UnsubscribeOnMaybeObserver(MaybeObserver<? super T> actual, Scheduler scheduler) {
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

        @Override
        public void onComplete() {
            actual.onComplete();
        }
    }
}
