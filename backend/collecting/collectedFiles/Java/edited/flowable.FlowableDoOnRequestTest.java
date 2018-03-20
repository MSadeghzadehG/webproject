

package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.*;

import java.util.*;
import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.*;

import io.reactivex.Flowable;
import io.reactivex.functions.*;
import io.reactivex.subscribers.DefaultSubscriber;

public class FlowableDoOnRequestTest {

    @Test
    public void testUnsubscribeHappensAgainstParent() {
        final AtomicBoolean unsubscribed = new AtomicBoolean(false);
        Flowable.just(1).concatWith(Flowable.<Integer>never())
                        .doOnCancel(new Action() {
                    @Override
                    public void run() {
                        unsubscribed.set(true);
                    }
                })
                                .doOnRequest(new LongConsumer() {
                    @Override
                    public void accept(long n) {
                                            }
                })
                                .subscribe().dispose();
        assertTrue(unsubscribed.get());
    }

    @Test
    public void testDoRequest() {
        final List<Long> requests = new ArrayList<Long>();
        Flowable.range(1, 5)
                        .doOnRequest(new LongConsumer() {
                    @Override
                    public void accept(long n) {
                        requests.add(n);
                    }
                })
                                .subscribe(new DefaultSubscriber<Integer>() {

                    @Override
                    public void onStart() {
                        request(3);
                    }

                    @Override
                    public void onComplete() {

                    }

                    @Override
                    public void onError(Throwable e) {

                    }

                    @Override
                    public void onNext(Integer t) {
                        request(t);
                    }
                });
        assertEquals(Arrays.asList(3L,1L,2L,3L,4L,5L), requests);
    }

    @Test
    @Ignore("This is a 1.x architecture-specific test")
    public void dontRequestIfDownstreamRequestsLate() {
    }
}
