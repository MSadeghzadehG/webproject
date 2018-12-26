

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;

public final class MaybeSubscribeOn<T> extends AbstractMaybeWithUpstream<T, T> {

    final Scheduler scheduler;

    public MaybeSubscribeOn(MaybeSource<T> source, Scheduler scheduler) {
        super(source);
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        SubscribeOnMaybeObserver<T> parent = new SubscribeOnMaybeObserver<T>(observer);
        observer.onSubscribe(parent);

        parent.task.replace(scheduler.scheduleDirect(new SubscribeTask<T>(parent, source)));
    }

    static final class SubscribeTask<T> implements Runnable {
        final MaybeObserver<? super T> observer;
        final MaybeSource<T> source;

        SubscribeTask(MaybeObserver<? super T> observer, MaybeSource<T> source) {
            this.observer = observer;
            this.source = source;
        }

        @Override
        public void run() {
            source.subscribe(observer);
        }
    }

    static final class SubscribeOnMaybeObserver<T>
    extends AtomicReference<Disposable>
    implements MaybeObserver<T>, Disposable {

        final SequentialDisposable task;

        private static final long serialVersionUID = 8571289934935992137L;

        final MaybeObserver<? super T> actual;

        SubscribeOnMaybeObserver(MaybeObserver<? super T> actual) {
            this.actual = actual;
            this.task = new SequentialDisposable();
        }

        @Override
        public void dispose() {
            DisposableHelper.dispose(this);
            task.dispose();
        }

        @Override
        public boolean isDisposed() {
            return DisposableHelper.isDisposed(get());
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(this, d);
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
