

package io.reactivex.internal.operators.completable;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.SequentialDisposable;

public final class CompletableConcatArray extends Completable {
    final CompletableSource[] sources;

    public CompletableConcatArray(CompletableSource[] sources) {
        this.sources = sources;
    }

    @Override
    public void subscribeActual(CompletableObserver s) {
        ConcatInnerObserver inner = new ConcatInnerObserver(s, sources);
        s.onSubscribe(inner.sd);
        inner.next();
    }

    static final class ConcatInnerObserver extends AtomicInteger implements CompletableObserver {

        private static final long serialVersionUID = -7965400327305809232L;

        final CompletableObserver actual;
        final CompletableSource[] sources;

        int index;

        final SequentialDisposable sd;

        ConcatInnerObserver(CompletableObserver actual, CompletableSource[] sources) {
            this.actual = actual;
            this.sources = sources;
            this.sd = new SequentialDisposable();
        }

        @Override
        public void onSubscribe(Disposable d) {
            sd.replace(d);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            next();
        }

        void next() {
            if (sd.isDisposed()) {
                return;
            }

            if (getAndIncrement() != 0) {
                return;
            }

            CompletableSource[] a = sources;
            do {
                if (sd.isDisposed()) {
                    return;
                }

                int idx = index++;
                if (idx == a.length) {
                    actual.onComplete();
                    return;
                }

                a[idx].subscribe(this);
            } while (decrementAndGet() != 0);
        }
    }
}
