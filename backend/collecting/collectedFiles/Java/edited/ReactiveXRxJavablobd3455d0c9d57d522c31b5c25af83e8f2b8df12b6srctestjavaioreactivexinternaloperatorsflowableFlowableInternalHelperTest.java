
package io.reactivex.internal.operators.flowable;

import org.junit.Test;

import io.reactivex.TestHelper;

public class FlowableInternalHelperTest {

    @Test
    public void utilityClass() {
        TestHelper.checkUtilityClass(FlowableInternalHelper.class);
    }

    @Test
    public void requestMaxEnum() {
        TestHelper.checkEnum(FlowableInternalHelper.RequestMax.class);
    }
}
