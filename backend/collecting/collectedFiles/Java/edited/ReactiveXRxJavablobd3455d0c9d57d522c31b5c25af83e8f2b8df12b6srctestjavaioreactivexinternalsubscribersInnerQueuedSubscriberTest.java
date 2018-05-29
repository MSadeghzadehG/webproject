

package io.reactivex.internal.subscribers;

import java.util.*;

import static org.junit.Assert.*;
import org.junit.Test;
import org.reactivestreams.Subscription;

public class InnerQueuedSubscriberTest {

    @Test
    public void requestInBatches() {
        InnerQueuedSubscriberSupport<Integer> support = new InnerQueuedSubscriberSupport<Integer>() {
            @Override
            public void innerNext(InnerQueuedSubscriber<Integer> inner, Integer value) {
            }
            @Override
            public void innerError(InnerQueuedSubscriber<Integer> inner, Throwable e) {
            }
            @Override
            public void innerComplete(InnerQueuedSubscriber<Integer> inner) {
            }
            @Override
            public void drain() {
            }
        };

        InnerQueuedSubscriber<Integer> inner = new InnerQueuedSubscriber<Integer>(support, 4);

        final List<Long> requests = new ArrayList<Long>();

        inner.onSubscribe(new Subscription() {
            @Override
            public void request(long n) {
                requests.add(n);
            }
            @Override
            public void cancel() {
                            }
        });

        inner.request(1);
        inner.request(1);
        inner.request(1);
        inner.request(1);
        inner.request(1);

        assertEquals(Arrays.asList(4L, 3L), requests);
    }
}
