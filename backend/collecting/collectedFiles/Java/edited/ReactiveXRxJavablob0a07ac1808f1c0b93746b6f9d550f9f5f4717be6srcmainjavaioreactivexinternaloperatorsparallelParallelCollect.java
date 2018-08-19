

package io.reactivex.internal.operators.parallel;

import java.util.concurrent.Callable;

import org.reactivestreams.*;

import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BiConsumer;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscribers.DeferredScalarSubscriber;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.parallel.ParallelFlowable;
import io.reactivex.plugins.RxJavaPlugins;


public final class ParallelCollect<T, C> extends ParallelFlowable<C> {

    final ParallelFlowable<? extends T> source;

    final Callable<? extends C> initialCollection;

    final BiConsumer<? super C, ? super T> collector;

    public ParallelCollect(ParallelFlowable<? extends T> source,
            Callable<? extends C> initialCollection, BiConsumer<? super C, ? super T> collector) {
        this.source = source;
        this.initialCollection = initialCollection;
        this.collector = collector;
    }

    @Override
    public void subscribe(Subscriber<? super C>[] subscribers) {
        if (!validate(subscribers)) {
            return;
        }

        int n = subscribers.length;
        @SuppressWarnings("unchecked")
        Subscriber<T>[] parents = new Subscriber[n];

        for (int i = 0; i < n; i++) {

            C initialValue;

            try {
                initialValue = ObjectHelper.requireNonNull(initialCollection.call(), "The initialSupplier returned a null value");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                reportError(subscribers, ex);
                return;
            }

            parents[i] = new ParallelCollectSubscriber<T, C>(subscribers[i], initialValue, collector);
        }

        source.subscribe(parents);
    }

    void reportError(Subscriber<?>[] subscribers, Throwable ex) {
        for (Subscriber<?> s : subscribers) {
            EmptySubscription.error(ex, s);
        }
    }

    @Override
    public int parallelism() {
        return source.parallelism();
    }

    static final class ParallelCollectSubscriber<T, C> extends DeferredScalarSubscriber<T, C> {


        private static final long serialVersionUID = -4767392946044436228L;

        final BiConsumer<? super C, ? super T> collector;

        C collection;

        boolean done;

        ParallelCollectSubscriber(Subscriber<? super C> subscriber,
                C initialValue, BiConsumer<? super C, ? super T> collector) {
            super(subscriber);
            this.collection = initialValue;
            this.collector = collector;
        }

        @Override
        public void onSubscribe(Subscription s) {
            if (SubscriptionHelper.validate(this.s, s)) {
                this.s = s;

                actual.onSubscribe(this);

                s.request(Long.MAX_VALUE);
            }
        }

        @Override
        public void onNext(T t) {
            if (done) {
                return;
            }

            try {
                collector.accept(collection, t);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                cancel();
                onError(ex);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            collection = null;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            C c = collection;
            collection = null;
            complete(c);
        }

        @Override
        public void cancel() {
            super.cancel();
            s.cancel();
        }
    }
}
