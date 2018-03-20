

package io.reactivex.internal.operators.completable;

import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.*;
import io.reactivex.internal.functions.ObjectHelper;

public final class CompletableConcatIterable extends Completable {
    final Iterable<? extends CompletableSource> sources;

    public CompletableConcatIterable(Iterable<? extends CompletableSource> sources) {
        this.sources = sources;
    }

    @Override
    public void subscribeActual(CompletableObserver s) {

        Iterator<? extends CompletableSource> it;

        try {
            it = ObjectHelper.requireNonNull(sources.iterator(), "The iterator returned is null");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            EmptyDisposable.error(e, s);
            return;
        }

        ConcatInnerObserver inner = new ConcatInnerObserver(s, it);
        s.onSubscribe(inner.sd);
        inner.next();
    }

    static final class ConcatInnerObserver extends AtomicInteger implements CompletableObserver {

        private static final long serialVersionUID = -7965400327305809232L;

        final CompletableObserver actual;
        final Iterator<? extends CompletableSource> sources;

        final SequentialDisposable sd;

        ConcatInnerObserver(CompletableObserver actual, Iterator<? extends CompletableSource> sources) {
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

            Iterator<? extends CompletableSource> a = sources;
            do {
                if (sd.isDisposed()) {
                    return;
                }

                boolean b;
                try {
                    b = a.hasNext();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    actual.onError(ex);
                    return;
                }

                if (!b) {
                    actual.onComplete();
                    return;
                }

                CompletableSource c;

                try {
                    c = ObjectHelper.requireNonNull(a.next(), "The CompletableSource returned is null");
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    actual.onError(ex);
                    return;
                }

                c.subscribe(this);
            } while (decrementAndGet() != 0);
        }
    }
}
