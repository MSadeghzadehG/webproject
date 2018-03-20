
package io.reactivex.flowable;

import java.util.Arrays;

import org.junit.Test;

import io.reactivex.Flowable;
import io.reactivex.internal.fuseable.*;
import io.reactivex.subscribers.SubscriberFusion;

public class FlowableFuseableTest {

    @Test
    public void syncRange() {

        Flowable.range(1, 10)
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncArray() {

        Flowable.fromArray(new Integer[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 })
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncIterable() {

        Flowable.fromIterable(Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10))
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncRangeHidden() {

        Flowable.range(1, 10).hide()
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertNotFuseable())
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncArrayHidden() {
        Flowable.fromArray(new Integer[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 })
        .hide()
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertNotFuseable())
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncIterableHidden() {
        Flowable.fromIterable(Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10))
        .hide()
        .to(SubscriberFusion.<Integer>test(Long.MAX_VALUE, QueueFuseable.ANY, false))
        .assertOf(SubscriberFusion.<Integer>assertNotFuseable())
        .assertOf(SubscriberFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }
}
