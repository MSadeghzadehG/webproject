

package io.reactivex.internal.operators.flowable;

import java.util.Collection;
import java.util.concurrent.Callable;

import org.reactivestreams.Subscription;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.fuseable.FuseToFlowable;
import io.reactivex.internal.subscriptions.SubscriptionHelper;
import io.reactivex.internal.util.ArrayListSupplier;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableToListSingle<T, U extends Collection<? super T>> extends Single<U> implements FuseToFlowable<U> {

    final Flowable<T> source;

    final Callable<U> collectionSupplier;

    @SuppressWarnings("unchecked")
    public FlowableToListSingle(Flowable<T> source) {
        this(source, (Callable<U>)ArrayListSupplier.asCallable());
    }

    public FlowableToListSingle(Flowable<T> source, Callable<U> collectionSupplier) {
        this.source = source;
        this.collectionSupplier = collectionSupplier;
    }

    @Override
    protected void subscribeActual(SingleObserver<? super U> s) {
        U coll;
        try {
            coll = ObjectHelper.requireNonNull(collectionSupplier.call(), "The collectionSupplier returned a null collection. Null values are generally not allowed in 2.x operators and sources.");
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            EmptyDisposable.error(e, s);
            return;
        }
        source.subscribe(new ToListSubscriber<T, U>(s, coll));
    }

    @Override
    public Flowable<U> fuseToFlowable() {
        return RxJavaPlugins.onAssembly(new FlowableToList<T, U>(source, collectionSupplier));
    }

    static final class ToListSubscriber<T, U extends Collection<? super T>>
    implements FlowableSubscriber<T>, Disposable {

        final SingleObserver<? super U> actual;

        Subscription s;

        U value;

        ToListSubscriber(SingleObserver<? super U> actual, U collection) {
            this.actual = actual;
            this.value = collection;
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
            value.add(t);
        }

        @Override
        public void onError(Throwable t) {
            value = null;
            s = SubscriptionHelper.CANCELLED;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            s = SubscriptionHelper.CANCELLED;
            actual.onSuccess(value);
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
