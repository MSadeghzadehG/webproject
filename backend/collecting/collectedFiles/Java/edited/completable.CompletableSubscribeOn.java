

package io.reactivex.internal.operators.completable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;

public final class CompletableSubscribeOn extends Completable {
    final CompletableSource source;

    final Scheduler scheduler;

    public CompletableSubscribeOn(CompletableSource source, Scheduler scheduler) {
        this.source = source;
        this.scheduler = scheduler;
    }

    @Override
    protected void subscribeActual(final CompletableObserver s) {

        final SubscribeOnObserver parent = new SubscribeOnObserver(s, source);
        s.onSubscribe(parent);

        Disposable f = scheduler.scheduleDirect(parent);

        parent.task.replace(f);

    }

    static final class SubscribeOnObserver
    extends AtomicReference<Disposable>
    implements CompletableObserver, Disposable, Runnable {

        private static final long serialVersionUID = 7000911171163930287L;

        final CompletableObserver actual;

        final SequentialDisposable task;

        final CompletableSource source;

        SubscribeOnObserver(CompletableObserver actual, CompletableSource source) {
            this.actual = actual;
            this.source = source;
            this.task = new SequentialDisposable();
        }

        @Override
        public void run() {
            source.subscribe(this);
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(this, d);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
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
    }

}
