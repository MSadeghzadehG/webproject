

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.*;

import org.reactivestreams.*;

import io.reactivex.functions.*;
import io.reactivex.internal.functions.*;
import io.reactivex.internal.subscribers.*;
import io.reactivex.internal.util.*;


public final class FlowableBlockingSubscribe {

    
    private FlowableBlockingSubscribe() {
        throw new IllegalStateException("No instances!");
    }

    
    public static <T> void subscribe(Publisher<? extends T> o, Subscriber<? super T> subscriber) {
        final BlockingQueue<Object> queue = new LinkedBlockingQueue<Object>();

        BlockingSubscriber<T> bs = new BlockingSubscriber<T>(queue);

        o.subscribe(bs);

        try {
            for (;;) {
                if (bs.isCancelled()) {
                    break;
                }
                Object v = queue.poll();
                if (v == null) {
                    if (bs.isCancelled()) {
                        break;
                    }
                    BlockingHelper.verifyNonBlocking();
                    v = queue.take();
                }
                if (bs.isCancelled()) {
                    break;
                }
                if (o == BlockingSubscriber.TERMINATED
                        || NotificationLite.acceptFull(v, subscriber)) {
                    break;
                }
            }
        } catch (InterruptedException e) {
            bs.cancel();
            subscriber.onError(e);
        }
    }

    
    public static <T> void subscribe(Publisher<? extends T> o) {
        BlockingIgnoringReceiver callback = new BlockingIgnoringReceiver();
        LambdaSubscriber<T> ls = new LambdaSubscriber<T>(Functions.emptyConsumer(),
        callback, callback, Functions.REQUEST_MAX);

        o.subscribe(ls);

        BlockingHelper.awaitForComplete(callback, ls);
        Throwable e = callback.error;
        if (e != null) {
            throw ExceptionHelper.wrapOrThrow(e);
        }
    }

    
    public static <T> void subscribe(Publisher<? extends T> o, final Consumer<? super T> onNext,
            final Consumer<? super Throwable> onError, final Action onComplete) {
        ObjectHelper.requireNonNull(onNext, "onNext is null");
        ObjectHelper.requireNonNull(onError, "onError is null");
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");
        subscribe(o, new LambdaSubscriber<T>(onNext, onError, onComplete, Functions.REQUEST_MAX));
    }
}
