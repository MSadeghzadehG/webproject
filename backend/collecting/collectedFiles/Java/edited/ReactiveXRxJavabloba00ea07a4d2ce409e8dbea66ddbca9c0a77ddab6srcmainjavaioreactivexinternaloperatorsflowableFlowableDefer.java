

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.Callable;

import org.reactivestreams.*;

import io.reactivex.Flowable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.EmptySubscription;

public final class FlowableDefer<T> extends Flowable<T> {
    final Callable<? extends Publisher<? extends T>> supplier;
    public FlowableDefer(Callable<? extends Publisher<? extends T>> supplier) {
        this.supplier = supplier;
    }
    @Override
    public void subscribeActual(Subscriber<? super T> s) {
        Publisher<? extends T> pub;
        try {
            pub = ObjectHelper.requireNonNull(supplier.call(), "The publisher supplied is null");
        } catch (Throwable t) {
            Exceptions.throwIfFatal(t);
            EmptySubscription.error(t, s);
            return;
        }

        pub.subscribe(s);
    }
}
