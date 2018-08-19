
package io.reactivex.observable;

import java.util.Arrays;

import org.junit.Test;

import io.reactivex.Observable;
import io.reactivex.internal.fuseable.QueueFuseable;
import io.reactivex.observers.ObserverFusion;

public class ObservableFuseableTest {

    @Test
    public void syncRange() {

        Observable.range(1, 10)
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncArray() {

        Observable.fromArray(new Integer[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 })
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncIterable() {

        Observable.fromIterable(Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10))
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.SYNC))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncRangeHidden() {

        Observable.range(1, 10).hide()
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertNotFuseable())
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncArrayHidden() {
        Observable.fromArray(new Integer[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 })
        .hide()
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertNotFuseable())
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void syncIterableHidden() {
        Observable.fromIterable(Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9, 10))
        .hide()
        .to(ObserverFusion.<Integer>test(QueueFuseable.ANY, false))
        .assertOf(ObserverFusion.<Integer>assertNotFuseable())
        .assertOf(ObserverFusion.<Integer>assertFusionMode(QueueFuseable.NONE))
        .assertValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .assertNoErrors()
        .assertComplete();
    }
}
