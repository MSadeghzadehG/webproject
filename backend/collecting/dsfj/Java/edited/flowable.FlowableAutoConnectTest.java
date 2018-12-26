

package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.assertTrue;

import org.junit.Test;

import io.reactivex.processors.PublishProcessor;

public class FlowableAutoConnectTest {

    @Test
    public void autoConnectImmediately() {
        PublishProcessor<Integer> pp = PublishProcessor.create();

        pp.publish().autoConnect(0);

        assertTrue(pp.hasSubscribers());
    }
}
