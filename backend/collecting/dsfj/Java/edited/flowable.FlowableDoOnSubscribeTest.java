

package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.atomic.*;

import org.junit.Test;
import org.reactivestreams.*;

import io.reactivex.Flowable;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.subscriptions.BooleanSubscription;

public class FlowableDoOnSubscribeTest {

    @Test
    public void testDoOnSubscribe() throws Exception {
        final AtomicInteger count = new AtomicInteger();
        Flowable<Integer> o = Flowable.just(1).doOnSubscribe(new Consumer<Subscription>() {
            @Override
            public void accept(Subscription s) {
                    count.incrementAndGet();
            }
        });

        o.subscribe();
        o.subscribe();
        o.subscribe();
        assertEquals(3, count.get());
    }

    @Test
    public void testDoOnSubscribe2() throws Exception {
        final AtomicInteger count = new AtomicInteger();
        Flowable<Integer> o = Flowable.just(1).doOnSubscribe(new Consumer<Subscription>() {
            @Override
            public void accept(Subscription s) {
                    count.incrementAndGet();
            }
        }).take(1).doOnSubscribe(new Consumer<Subscription>() {
            @Override
            public void accept(Subscription s) {
                    count.incrementAndGet();
            }
        });

        o.subscribe();
        assertEquals(2, count.get());
    }

    @Test
    public void testDoOnUnSubscribeWorksWithRefCount() throws Exception {
        final AtomicInteger onSubscribed = new AtomicInteger();
        final AtomicInteger countBefore = new AtomicInteger();
        final AtomicInteger countAfter = new AtomicInteger();
        final AtomicReference<Subscriber<? super Integer>> sref = new AtomicReference<Subscriber<? super Integer>>();
        Flowable<Integer> o = Flowable.unsafeCreate(new Publisher<Integer>() {

            @Override
            public void subscribe(Subscriber<? super Integer> s) {
                s.onSubscribe(new BooleanSubscription());
                onSubscribed.incrementAndGet();
                sref.set(s);
            }

        }).doOnSubscribe(new Consumer<Subscription>() {
            @Override
            public void accept(Subscription s) {
                    countBefore.incrementAndGet();
            }
        }).publish().refCount()
        .doOnSubscribe(new Consumer<Subscription>() {
            @Override
            public void accept(Subscription s) {
                    countAfter.incrementAndGet();
            }
        });

        o.subscribe();
        o.subscribe();
        o.subscribe();
        assertEquals(1, countBefore.get());
        assertEquals(1, onSubscribed.get());
        assertEquals(3, countAfter.get());
        sref.get().onComplete();
        o.subscribe();
        o.subscribe();
        o.subscribe();
        assertEquals(2, countBefore.get());
        assertEquals(2, onSubscribed.get());
        assertEquals(6, countAfter.get());
    }

}
