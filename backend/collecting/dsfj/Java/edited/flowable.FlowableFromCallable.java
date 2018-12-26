

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.Callable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.DeferredScalarSubscription;

public final class FlowableFromCallable<T> extends Flowable<T> implements Callable<T> {
    final Callable<? extends T> callable;
    public FlowableFromCallable(Callable<? extends T> callable) {
        this.callable = callable;
    }
    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        DeferredScalarSubscription<T> deferred = new DeferredScalarSubscription<T>(s);
        s.onSubscribe(deferred);

        T t;
        try {
            t = ObjectHelper.requireNonNull(callable.call(), "The callable returned a null value");
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            s.onError(ex);
            return;
        }

        deferred.complete(t);
    }

    @Override
    public T call() throws Exception {
        return ObjectHelper.requireNonNull(callable.call(), "The callable returned a null value");
    }
}
