

package io.reactivex.internal.operators.observable;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.internal.operators.observable.ObservableInterval.IntervalObserver;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.*;

public class ObservableIntervalTest {

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Observable.interval(1, TimeUnit.MILLISECONDS, new TestScheduler()));
    }

    @Test(timeout = 2000)
    public void cancel() {
        Observable.interval(1, TimeUnit.MILLISECONDS, Schedulers.trampoline())
        .take(10)
        .test()
        .assertResult(0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L);
    }

    @Test
    public void cancelledOnRun() {
        TestObserver<Long> to = new TestObserver<Long>();
        IntervalObserver is = new IntervalObserver(to);
        to.onSubscribe(is);

        is.dispose();

        is.run();

        to.assertEmpty();
    }
}
