

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.annotations.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Action;
import io.reactivex.internal.fuseable.*;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;


@Experimental
public final class FlowableDoFinally<T> extends AbstractFlowableWithUpstream<T, T> {

    final Action onFinally;

    public FlowableDoFinally(Flowable<T> source, Action onFinally) {
        super(source);
        this.onFinally = onFinally;
    }

    @Override
    protected void subscribeActual(Subscriber<? super T> s) {
        if (s instanceof ConditionalSubscriber) {
            source.subscribe(new DoFinallyConditionalSubscriber<T>((ConditionalSubscriber<? super T>)s, onFinally));
        } else {
            source.subscribe(new DoFinallySubscriber<T>(s, onFinally));
        }
    }

    static final class DoFinallySubscriber<T> extends BasicIntQueueSubscription<T> implements FlowableSubscriber<T> {

        private static final long serialVersionUID = 4109457741734051389L;

        final Subscriber<? super T> actual;

        final Action onFinally;

        Subscription s;

        QueueSubscription<T> qs;

        boolean syncFused;

        DoFinallySubscriber(Subscriber<? super T> actual, Action onFinally) {
            this.actual = actual;
            this.onFinally = onFinally;
        }

        @SuppressWarnings("unchecked")
        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;
                if (s instanceof QueueSubscription) {
                    this.qs = (QueueSubscription<T>)s;
                }

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
            runFinally();
        }

        @Override
        public void onComplete() {
            actual.onComplete();
            runFinally();
        }

        @Override
        public void cancel() {
            s.cancel();
            runFinally();
        }

        @Override
        public void request(long n) {
            s.request(n);
        }

        @Override
        public int requestFusion(int mode) {
            QueueSubscription<T> qs = this.qs;
            if (qs != null && (mode & BOUNDARY) == 0) {
                int m = qs.requestFusion(mode);
                if (m != NONE) {
                    syncFused = m == SYNC;
                }
                return m;
            }
            return NONE;
        }

        @Override
        public void clear() {
            qs.clear();
        }

        @Override
        public boolean isEmpty() {
            return qs.isEmpty();
        }

        @Nullable
        @Override
        public T poll() throws Exception {
            T v = qs.poll();
            if (v == null && syncFused) {
                runFinally();
            }
            return v;
        }

        void runFinally() {
            if (compareAndSet(0, 1)) {
                try {
                    onFinally.run();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
            }
        }
    }

    static final class DoFinallyConditionalSubscriber<T> extends BasicIntQueueSubscription<T> implements ConditionalSubscriber<T> {

        private static final long serialVersionUID = 4109457741734051389L;

        final ConditionalSubscriber<? super T> actual;

        final Action onFinally;

        Subscription s;

        QueueSubscription<T> qs;

        boolean syncFused;

        DoFinallyConditionalSubscriber(ConditionalSubscriber<? super T> actual, Action onFinally) {
            this.actual = actual;
            this.onFinally = onFinally;
        }

        @SuppressWarnings("unchecked")
        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;
                if (s instanceof QueueSubscription) {
                    this.qs = (QueueSubscription<T>)s;
                }

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public boolean tryOnNext(T t) {
            return actual.tryOnNext(t);
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
            runFinally();
        }

        @Override
        public void onComplete() {
            actual.onComplete();
            runFinally();
        }

        @Override
        public void cancel() {
            s.cancel();
            runFinally();
        }

        @Override
        public void request(long n) {
            s.request(n);
        }

        @Override
        public int requestFusion(int mode) {
            QueueSubscription<T> qs = this.qs;
            if (qs != null && (mode & BOUNDARY) == 0) {
                int m = qs.requestFusion(mode);
                if (m != NONE) {
                    syncFused = m == SYNC;
                }
                return m;
            }
            return NONE;
        }

        @Override
        public void clear() {
            qs.clear();
        }

        @Override
        public boolean isEmpty() {
            return qs.isEmpty();
        }

        @Nullable
        @Override
        public T poll() throws Exception {
            T v = qs.poll();
            if (v == null && syncFused) {
                runFinally();
            }
            return v;
        }

        void runFinally() {
            if (compareAndSet(0, 1)) {
                try {
                    onFinally.run();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
            }
        }
    }
}
