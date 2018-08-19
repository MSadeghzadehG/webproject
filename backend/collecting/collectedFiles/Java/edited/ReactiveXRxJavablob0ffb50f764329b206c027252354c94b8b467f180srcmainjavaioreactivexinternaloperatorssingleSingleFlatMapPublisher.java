

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.Publisher;
import org.reactivestreams.Subscriber;
import org.reactivestreams.Subscription;

import io.reactivex.Flowable;
import io.reactivex.FlowableSubscriber;
import io.reactivex.Scheduler;
import io.reactivex.SingleObserver;
import io.reactivex.SingleSource;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.SubscriptionHelper;


public final class SingleFlatMapPublisher<T, R> extends Flowable<R> {

    final SingleSource<T> source;
    final Function<? super T, ? extends Publisher<? extends R>> mapper;

    public SingleFlatMapPublisher(SingleSource<T> source,
            Function<? super T, ? extends Publisher<? extends R>> mapper) {
        this.source = source;
        this.mapper = mapper;
    }

    @Override
    protected void subscribeActual(Subscriber<? super R> actual) {
        source.subscribe(new SingleFlatMapPublisherObserver<T, R>(actual, mapper));
    }

    static final class SingleFlatMapPublisherObserver<S, T> extends AtomicLong
            implements SingleObserver<S>, FlowableSubscriber<T>, Subscription {

        private static final long serialVersionUID = 7759721921468635667L;

        final Subscriber<? super T> actual;
        final Function<? super S, ? extends Publisher<? extends T>> mapper;
        final AtomicReference<Subscription> parent;
        Disposable disposable;

        SingleFlatMapPublisherObserver(Subscriber<? super T> actual,
                Function<? super S, ? extends Publisher<? extends T>> mapper) {
            this.actual = actual;
            this.mapper = mapper;
            this.parent = new AtomicReference<Subscription>();
        }

        @Override
        public void onSubscribe(Disposable d) {
            this.disposable = d;
            actual.onSubscribe(this);
        }

        @Override
        public void onSuccess(S value) {
            Publisher<? extends T> f;
            try {
                f = ObjectHelper.requireNonNull(mapper.apply(value), "the mapper returned a null Publisher");
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                actual.onError(e);
                return;
            }
            f.subscribe(this);
        }

        @Override
        public void onSubscribe(Subscription s) {
            SubscriptionHelper.deferredSetOnce(parent, this, s);
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void request(long n) {
            SubscriptionHelper.deferredRequest(parent, this, n);
        }

        @Override
        public void cancel() {
            disposable.dispose();
            SubscriptionHelper.cancel(parent);
        }
    }

}
