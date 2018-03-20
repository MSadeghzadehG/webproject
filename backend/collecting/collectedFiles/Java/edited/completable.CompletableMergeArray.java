

package io.reactivex.internal.operators.completable;

import java.util.concurrent.atomic.*;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.plugins.RxJavaPlugins;

public final class CompletableMergeArray extends Completable {
    final CompletableSource[] sources;

    public CompletableMergeArray(CompletableSource[] sources) {
        this.sources = sources;
    }

    @Override
    public void subscribeActual(final CompletableObserver s) {
        final CompositeDisposable set = new CompositeDisposable();
        final AtomicBoolean once = new AtomicBoolean();

        InnerCompletableObserver shared = new InnerCompletableObserver(s, once, set, sources.length + 1);
        s.onSubscribe(set);

        for (CompletableSource c : sources) {
            if (set.isDisposed()) {
                return;
            }

            if (c == null) {
                set.dispose();
                NullPointerException npe = new NullPointerException("A completable source is null");
                shared.onError(npe);
                return;
            }

            c.subscribe(shared);
        }

        shared.onComplete();
    }

    static final class InnerCompletableObserver extends AtomicInteger implements CompletableObserver {
        private static final long serialVersionUID = -8360547806504310570L;

        final CompletableObserver actual;

        final AtomicBoolean once;

        final CompositeDisposable set;

        InnerCompletableObserver(CompletableObserver actual, AtomicBoolean once, CompositeDisposable set, int n) {
            this.actual = actual;
            this.once = once;
            this.set = set;
            this.lazySet(n);
        }

        @Override
        public void onSubscribe(Disposable d) {
            set.add(d);
        }

        @Override
        public void onError(Throwable e) {
            set.dispose();
            if (once.compareAndSet(false, true)) {
                actual.onError(e);
            } else {
                RxJavaPlugins.onError(e);
            }
        }

        @Override
        public void onComplete() {
            if (decrementAndGet() == 0) {
                if (once.compareAndSet(false, true)) {
                    actual.onComplete();
                }
            }
        }
    }
}
