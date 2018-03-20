

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.internal.operators.flowable.FlowableInterval.IntervalSubscriber;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subscribers.TestSubscriber;

public class FlowableIntervalTest {

    @Test(timeout = 2000)
    public void cancel() {
        Flowable.interval(1, TimeUnit.MILLISECONDS, Schedulers.trampoline())
        .take(10)
        .test()
        .assertResult(0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L);
    }

    @Test
    public void badRequest() {
        TestHelper.assertBadRequestReported(Flowable.interval(1, TimeUnit.MILLISECONDS, Schedulers.trampoline()));
    }

    @Test
    public void cancelledOnRun() {
        TestSubscriber<Long> ts = new TestSubscriber<Long>();
        IntervalSubscriber is = new IntervalSubscriber(ts);
        ts.onSubscribe(is);

        is.cancel();

        is.run();

        ts.assertEmpty();
    }
}
