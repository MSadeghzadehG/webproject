

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.*;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.SequentialDisposable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.*;


public final class MaybeConcatArray<T> extends Flowable<T> {

    final MaybeSource<? extends T>[] sources;

    public MaybeConcatArray(MaybeSource<? extends T>[] sources) {
        this.sources = sources;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        ConcatMaybeObserver<T> parent = new ConcatMaybeObserver<T>(s, sources);
        s.onSubscribe(parent);
        parent.drain();
    }

    static final class ConcatMaybeObserver<T>
    extends AtomicInteger
    implements MaybeObserver<T>, Subscription {

        private static final long serialVersionUID = 3520831347801429610L;

        final Subscriber<? super T> actual;

        final AtomicLong requested;

        final AtomicReference<Object> current;

        final SequentialDisposable disposables;

        final MaybeSource<? extends T>[] sources;

        int index;

        long produced;

        ConcatMaybeObserver(Subscriber<? super T> actual, MaybeSource<? extends T>[] sources) {
            this.actual = actual;
            this.sources = sources;
            this.requested = new AtomicLong();
            this.disposables = new SequentialDisposable();
            this.current = new AtomicReference<Object>(NotificationLite.COMPLETE);         }

        @Override
        public void request(long n) {
            if (SubscriptionHelper.validate(n)) {
                BackpressureHelper.add(requested, n);
                drain();
            }
        }

        @Override
        public void cancel() {
            disposables.dispose();
        }

        @Override
        public void onSubscribe(Disposable d) {
            disposables.replace(d);
        }

        @Override
        public void onSuccess(T value) {
            current.lazySet(value);
            drain();
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            current.lazySet(NotificationLite.COMPLETE);
            drain();
        }

        @SuppressWarnings("unchecked")
        void drain() {
            if (getAndIncrement() != 0) {
                return;
            }

            AtomicReference<Object> c = current;
            Subscriber<? super T> a = actual;
            Disposable cancelled = disposables;

            for (;;) {
                if (cancelled.isDisposed()) {
                    c.lazySet(null);
                    return;
                }

                Object o = c.get();

                if (o != null) {
                    boolean goNextSource;
                    if (o != NotificationLite.COMPLETE) {
                        long p = produced;
                        if (p != requested.get()) {
                            produced = p + 1;
                            c.lazySet(null);
                            goNextSource = true;

                            a.onNext((T)o);
                        } else {
                            goNextSource = false;
                        }
                    } else {
                        goNextSource = true;
                        c.lazySet(null);
                    }

                    if (goNextSource && !cancelled.isDisposed()) {
                        int i = index;
                        if (i == sources.length) {
                            a.onComplete();
                            return;
                        }
                        index = i + 1;

                        sources[i].subscribe(this);
                    }
                }

                if (decrementAndGet() == 0) {
                    break;
                }
            }
        }
    }
}
