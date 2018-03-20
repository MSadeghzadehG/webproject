

package io.reactivex.flowable;

import org.junit.Test;

import io.reactivex.TestHelper;

public class FlowableEventStreamTest {
    @Test
    public void constructorShouldBePrivate() {
        TestHelper.checkUtilityClass(FlowableEventStream.class);
    }
}
