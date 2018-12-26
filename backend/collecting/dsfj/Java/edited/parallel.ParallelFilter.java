

package io.reactivex.internal.operators.parallel;

import org.reactivestreams.*;

import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.fuseable.ConditionalSubscriber;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.parallel.ParallelFlowable;
import io.reactivex.plugins.RxJavaPlugins;


public final class ParallelFilter<T> extends ParallelFlowable<T> {

    final ParallelFlowable<T> source;

    final Predicate<? super T> predicate;

    public ParallelFilter(ParallelFlowable<T> source, Predicate<? super T> predicate) {
        this.source = source;
        this.predicate = predicate;
    }

    @Override
    public void subscribe(Subscriber<? super T>[] subscribers) {
        if (!validate(subscribers)) {
            return;
        }

        int n = subscribers.length;
        @SuppressWarnings("unchecked")
        Subscriber<? super T>[] parents = new Subscriber[n];

        for (int i = 0; i < n; i++) {
            Subscriber<? super T> a = subscribers[i];
            if (a instanceof ConditionalSubscriber) {
                parents[i] = new ParallelFilterConditionalSubscriber<T>((ConditionalSubscriber<? super T>)a, predicate);
            } else {
                parents[i] = new ParallelFilterSubscriber<T>(a, predicate);
            }
        }

        source.subscribe(parents);
    }

    @Override
    public int parallelism() {
        return source.parallelism();
    }

    abstract static class BaseFilterSubscriber<T> implements ConditionalSubscriber<T>, Subscription {
        final Predicate<? super T> predicate;

        Subscription s;

        boolean done;

        BaseFilterSubscriber(Predicate<? super T> predicate) {
            this.predicate = predicate;
        }

        @Override
        public final void request(long n) {
            s.request(n);
        }

        @Override
        public final void cancel() {
            s.cancel();
        }

        @Override
        public final void onNext(T t) {
            if (!tryOnNext(t) && !done) {
                s.request(1);
            }
        }
    }

    static final class ParallelFilterSubscriber<T> extends BaseFilterSubscriber<T> {

        final Subscriber<? super T> actual;

        ParallelFilterSubscriber(Subscriber<? super T> actual, Predicate<? super T> predicate) {
            super(predicate);
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;

                actual.onSubscribe(this);
            }
        }

        @Override
        public boolean tryOnNext(T t) {
            if (!done) {
                boolean b;

                try {
                    b = predicate.test(t);
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    cancel();
                    onError(ex);
                    return false;
                }

                if (b) {
                    actual.onNext(t);
                    return true;
                }
            }
            return false;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (!done) {
                done = true;
                actual.onComplete();
            }
        }
    }

    static final class ParallelFilterConditionalSubscriber<T> extends BaseFilterSubscriber<T> {

        final ConditionalSubscriber<? super T> actual;

        ParallelFilterConditionalSubscriber(ConditionalSubscriber<? super T> actual, Predicate<? super T> predicate) {
            super(predicate);
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;

                actual.onSubscribe(this);
            }
        }

        @Override
        public boolean tryOnNext(T t) {
            if (!done) {
                boolean b;

                try {
                    b = predicate.test(t);
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    cancel();
                    onError(ex);
                    return false;
                }

                if (b) {
                    return actual.tryOnNext(t);
                }
            }
            return false;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (!done) {
                done = true;
                actual.onComplete();
            }
        }
    }}
