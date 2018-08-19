

package io.reactivex.internal.operators.single;

import java.util.Iterator;
import java.util.concurrent.atomic.AtomicLong;

import io.reactivex.annotations.Nullable;
import org.reactivestreams.Subscriber;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.internal.util.BackpressureHelper;


public final class SingleFlatMapIterableFlowable<T, R> extends Flowable<R> {

    final SingleSource<T> source;

    final Function<? super T, ? extends Iterable<? extends R>> mapper;

    public SingleFlatMapIterableFlowable(SingleSource<T> source,
            Function<? super T, ? extends Iterable<? extends R>> mapper) {
        this.source = source;
        this.mapper = mapper;
    }

    @Override
    protected void subscribeActual(Subscriber<? super R> s) {
        source.subscribe(new FlatMapIterableObserver<T, R>(s, mapper));
    }

    static final class FlatMapIterableObserver<T, R>
    extends BasicIntQueueSubscription<R>
    implements SingleObserver<T> {

        private static final long serialVersionUID = -8938804753851907758L;

        final Subscriber<? super R> actual;

        final Function<? super T, ? extends Iterable<? extends R>> mapper;

        final AtomicLong requested;

        Disposable d;

        volatile Iterator<? extends R> it;

        volatile boolean cancelled;

        boolean outputFused;

        FlatMapIterableObserver(Subscriber<? super R> actual,
                Function<? super T, ? extends Iterable<? extends R>> mapper) {
            this.actual = actual;
            this.mapper = mapper;
            this.requested = new AtomicLong();
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            Iterator<? extends R> iterator;
            boolean has;
            try {
                iterator = mapper.apply(value).iterator();

                has = iterator.hasNext();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                actual.onError(ex);
                return;
            }

            if (!has) {
                actual.onComplete();
                return;
            }

            this.it = iterator;
            drain();
        }

        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            actual.onError(e);
        }

        @Override
        public void request(long n) {
            if (SubscriptionHelper.validate(n)) {
                BackpressureHelper.add(requested, n);
                drain();
            }
        }

        @Override
        public void cancel() {
            cancelled = true;
            d.dispose();
            d = DisposableHelper.DISPOSED;
        }

        void drain() {
            if (getAndIncrement() != 0) {
                return;
            }

            Subscriber<? super R> a = actual;
            Iterator<? extends R> iterator = this.it;

            if (outputFused && iterator != null) {
                a.onNext(null);
                a.onComplete();
                return;
            }

            int missed = 1;

            for (;;) {

                if (iterator != null) {
                    long r = requested.get();
                    long e = 0L;

                    if (r == Long.MAX_VALUE) {
                        slowPath(a, iterator);
                        return;
                    }

                    while (e != r) {
                        if (cancelled) {
                            return;
                        }

                        R v;

                        try {
                            v = ObjectHelper.requireNonNull(iterator.next(), "The iterator returned a null value");
                        } catch (Throwable ex) {
                            Exceptions.throwIfFatal(ex);
                            a.onError(ex);
                            return;
                        }

                        a.onNext(v);

                        if (cancelled) {
                            return;
                        }

                        e++;

                        boolean b;

                        try {
                            b = iterator.hasNext();
                        } catch (Throwable ex) {
                            Exceptions.throwIfFatal(ex);
                            a.onError(ex);
                            return;
                        }

                        if (!b) {
                            a.onComplete();
                            return;
                        }
                    }

                    if (e != 0L) {
                        BackpressureHelper.produced(requested, e);
                    }
                }

                missed = addAndGet(-missed);
                if (missed == 0) {
                    break;
                }

                if (iterator == null) {
                    iterator = it;
                }
            }
        }

        void slowPath(Subscriber<? super R> a, Iterator<? extends R> iterator) {
            for (;;) {
                if (cancelled) {
                    return;
                }

                R v;

                try {
                    v = iterator.next();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    a.onError(ex);
                    return;
                }

                a.onNext(v);

                if (cancelled) {
                    return;
                }


                boolean b;

                try {
                    b = iterator.hasNext();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    a.onError(ex);
                    return;
                }

                if (!b) {
                    a.onComplete();
                    return;
                }
            }
        }

        @Override
        public int requestFusion(int mode) {
            if ((mode & ASYNC) != 0) {
                outputFused = true;
                return ASYNC;
            }
            return NONE;
        }

        @Override
        public void clear() {
            it = null;
        }

        @Override
        public boolean isEmpty() {
            return it == null;
        }

        @Nullable
        @Override
        public R poll() throws Exception {
            Iterator<? extends R> iterator = it;

            if (iterator != null) {
                R v = ObjectHelper.requireNonNull(iterator.next(), "The iterator returned a null value");
                if (!iterator.hasNext()) {
                    it = null;
                }
                return v;
            }
            return null;
        }

    }
}
