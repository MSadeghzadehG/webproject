

package io.reactivex.internal.operators.observable;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.queue.SpscLinkedArrayQueue;

public final class ObservableTakeLastTimed<T> extends AbstractObservableWithUpstream<T, T> {
    final long count;
    final long time;
    final TimeUnit unit;
    final Scheduler scheduler;
    final int bufferSize;
    final boolean delayError;

    public ObservableTakeLastTimed(ObservableSource<T> source,
            long count, long time, TimeUnit unit, Scheduler scheduler, int bufferSize, boolean delayError) {
        super(source);
        this.count = count;
        this.time = time;
        this.unit = unit;
        this.scheduler = scheduler;
        this.bufferSize = bufferSize;
        this.delayError = delayError;
    }

    @Override
    public void subscribeActual(Observer<? super T> t) {
        source.subscribe(new TakeLastTimedObserver<T>(t, count, time, unit, scheduler, bufferSize, delayError));
    }

    static final class TakeLastTimedObserver<T>
    extends AtomicBoolean implements Observer<T>, Disposable {

        private static final long serialVersionUID = -5677354903406201275L;
        final Observer<? super T> actual;
        final long count;
        final long time;
        final TimeUnit unit;
        final Scheduler scheduler;
        final SpscLinkedArrayQueue<Object> queue;
        final boolean delayError;

        Disposable d;

        volatile boolean cancelled;

        Throwable error;

        TakeLastTimedObserver(Observer<? super T> actual, long count, long time, TimeUnit unit, Scheduler scheduler, int bufferSize, boolean delayError) {
            this.actual = actual;
            this.count = count;
            this.time = time;
            this.unit = unit;
            this.scheduler = scheduler;
            this.queue = new SpscLinkedArrayQueue<Object>(bufferSize);
            this.delayError = delayError;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            final SpscLinkedArrayQueue<Object> q = queue;

            long now = scheduler.now(unit);
            long time = this.time;
            long c = count;
            boolean unbounded = c == Long.MAX_VALUE;

            q.offer(now, t);

            while (!q.isEmpty()) {
                long ts = (Long)q.peek();
                if (ts <= now - time || (!unbounded && (q.size() >> 1) > c)) {
                    q.poll();
                    q.poll();
                } else {
                    break;
                }
            }
        }

        @Override
        public void onError(Throwable t) {
            error = t;
            drain();
        }

        @Override
        public void onComplete() {
            drain();
        }

        @Override
        public void dispose() {
            if (!cancelled) {
                cancelled = true;
                d.dispose();

                if (compareAndSet(false, true)) {
                    queue.clear();
                }
            }
        }

        @Override
        public boolean isDisposed() {
            return cancelled;
        }

        void drain() {
            if (!compareAndSet(false, true)) {
                return;
            }

            final Observer<? super T> a = actual;
            final SpscLinkedArrayQueue<Object> q = queue;
            final boolean delayError = this.delayError;

            for (;;) {
                if (cancelled) {
                    q.clear();
                    return;
                }

                if (!delayError) {
                    Throwable ex = error;
                    if (ex != null) {
                        q.clear();
                        a.onError(ex);
                        return;
                    }
                }

                Object ts = q.poll();                 boolean empty = ts == null;

                if (empty) {
                    Throwable ex = error;
                    if (ex != null) {
                        a.onError(ex);
                    } else {
                        a.onComplete();
                    }
                    return;
                }

                @SuppressWarnings("unchecked")
                T o = (T)q.poll();

                if ((Long)ts < scheduler.now(unit) - time) {
                    continue;
                }

                a.onNext(o);
            }
        }
    }
}
