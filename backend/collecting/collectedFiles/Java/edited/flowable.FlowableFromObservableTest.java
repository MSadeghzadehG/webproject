

package io.reactivex.internal.operators.flowable;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;

public class FlowableFromObservableTest {
    @Test
    public void dispose() {
        TestHelper.checkDisposed(Observable.just(1).toFlowable(BackpressureStrategy.MISSING));
    }

    @Test
    public void error() {
        Observable.error(new TestException())
        .toFlowable(BackpressureStrategy.MISSING)
        .test()
        .assertFailure(TestException.class);
    }
}
