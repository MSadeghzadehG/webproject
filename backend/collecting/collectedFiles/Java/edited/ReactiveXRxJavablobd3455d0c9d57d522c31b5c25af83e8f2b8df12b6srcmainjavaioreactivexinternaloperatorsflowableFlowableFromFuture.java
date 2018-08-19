

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.*;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.subscriptions.DeferredScalarSubscription;

public final class FlowableFromFuture<T> extends Flowable<T> {
    final Future<? extends T> future;
    final long timeout;
    final TimeUnit unit;

    public FlowableFromFuture(Future<? extends T> future, long timeout, TimeUnit unit) {
        this.future = future;
        this.timeout = timeout;
        this.unit = unit;
    }

    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        DeferredScalarSubscription<T> deferred = new DeferredScalarSubscription<T>(s);
        s.onSubscribe(deferred);

        T v;
        try {
            v = unit != null ? future.get(timeout, unit) : future.get();
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            if (!deferred.isCancelled()) {
                s.onError(ex);
            }
            return;
        }
        if (v == null) {
            s.onError(new NullPointerException("The future returned null"));
        } else {
            deferred.complete(v);
        }
    }
}
