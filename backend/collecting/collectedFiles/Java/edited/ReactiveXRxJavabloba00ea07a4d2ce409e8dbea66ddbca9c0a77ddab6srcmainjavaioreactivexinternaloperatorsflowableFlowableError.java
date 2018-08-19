

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.Callable;

import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.EmptySubscription;

public final class FlowableError<T> extends Flowable<T> {
    final Callable<? extends Throwable> errorSupplier;
    public FlowableError(Callable<? extends Throwable> errorSupplier) {
        this.errorSupplier = errorSupplier;
    }
    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        Throwable error;
        try {
            error = ObjectHelper.requireNonNull(errorSupplier.call(), "Callable returned null throwable. Null values are generally not allowed in 2.x operators and sources.");
        } catch (Throwable t) {
            Exceptions.throwIfFatal(t);
            error = t;
        }
        EmptySubscription.error(error, s);
    }
}
