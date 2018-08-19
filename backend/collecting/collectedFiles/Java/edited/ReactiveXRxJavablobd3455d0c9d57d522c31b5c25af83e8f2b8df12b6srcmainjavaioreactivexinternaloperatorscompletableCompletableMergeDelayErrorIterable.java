

package io.reactivex.internal.operators.completable;

import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.operators.completable.CompletableMergeDelayErrorArray.MergeInnerCompletableObserver;
import io.reactivex.internal.util.AtomicThrowable;

public final class CompletableMergeDelayErrorIterable extends Completable {

    final Iterable<? extends CompletableSource> sources;

    public CompletableMergeDelayErrorIterable(Iterable<? extends CompletableSource> sources) {
        this.sources = sources;
    }

    @Override
    public void subscribeActual(final CompletableObserver s) {
        final CompositeDisposable set = new CompositeDisposable();

        s.onSubscribe(set);

        Iterator<? extends CompletableSource> iterator;

        try {
            iterator = ObjectHelper.requireNonNull(sources.iterator(), "The source iterator returned is null");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            s.onError(e);
            return;
        }

        final AtomicInteger wip = new AtomicInteger(1);

        final AtomicThrowable error = new AtomicThrowable();

        for (;;) {
            if (set.isDisposed()) {
                return;
            }

            boolean b;
            try {
                b = iterator.hasNext();
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                error.addThrowable(e);
                break;
            }

            if (!b) {
                break;
            }

            if (set.isDisposed()) {
                return;
            }

            CompletableSource c;

            try {
                c = ObjectHelper.requireNonNull(iterator.next(), "The iterator returned a null CompletableSource");
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                error.addThrowable(e);
                break;
            }

            if (set.isDisposed()) {
                return;
            }

            wip.getAndIncrement();

            c.subscribe(new MergeInnerCompletableObserver(s, set, error, wip));
        }

        if (wip.decrementAndGet() == 0) {
            Throwable ex = error.terminate();
            if (ex == null) {
                s.onComplete();
            } else {
                s.onError(ex);
            }
        }
    }
}
