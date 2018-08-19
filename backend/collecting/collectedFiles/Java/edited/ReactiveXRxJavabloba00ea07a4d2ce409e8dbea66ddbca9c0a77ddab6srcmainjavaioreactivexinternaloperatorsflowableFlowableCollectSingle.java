
package io.reactivex.internal.operators.flowable;

import java.util.concurrent.Callable;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BiConsumer;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.fuseable.FuseToFlowable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableCollectSingle<T, U> extends Single<U> implements FuseToFlowable<U> {

    final Flowable<T> source;

    final Callable<? extends U> initialSupplier;
    final BiConsumer<? super U, ? super T> collector;

    public FlowableCollectSingle(Flowable<T> source, Callable<? extends U> initialSupplier, BiConsumer<? super U, ? super T> collector) {
        this.source = source;
        this.initialSupplier = initialSupplier;
        this.collector = collector;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super U> s) {
        U u;
        try {
            u = ObjectHelper.requireNonNull(initialSupplier.call(), "The initialSupplier returned a null value");
        } catch (Throwable e) {
            EmptyDisposable.error(e, s);
            return;
        }

        source.subscribe(new CollectSubscriber<T, U>(s, u, collector));
    }

    @Override
    public Flowable<U> fuseToFlowable() {
        return RxJavaPlugins.onAssembly(new FlowableCollect<T, U>(source, initialSupplier, collector));
    }

    static final class CollectSubscriber<T, U> implements FlowableSubscriber<T>, Disposable {

        final SingleObserver<? super U> actual;

        final BiConsumer<? super U, ? super T> collector;

        final U u;

        Subscription s;

        boolean done;

        CollectSubscriber(SingleObserver<? super U> actual, U u, BiConsumer<? super U, ? super T> collector) {
            this.actual = actual;
            this.collector = collector;
            this.u = u;
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
                collector.accept(u, t);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                s.cancel();
                onError(e);
            }
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            s = SubscriptionHelper.CANCELLED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (done) {
                return;
            }
            done = true;
            s = SubscriptionHelper.CANCELLED;
            actual.onSuccess(u);
        }

        @Override
        public void dispose() {
            s.cancel();
            s = SubscriptionHelper.CANCELLED;
        }

        @Override
        public boolean isDisposed() {
            return s == SubscriptionHelper.CANCELLED;
        }
    }
}
