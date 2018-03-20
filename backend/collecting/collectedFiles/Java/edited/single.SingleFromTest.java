

package io.reactivex.internal.operators.single;

import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.schedulers.Schedulers;

public class SingleFromTest {

    @Test
    public void fromFuture() throws Exception {
        Single.fromFuture(Flowable.just(1).toFuture(), Schedulers.io())
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(1);
    }

    @Test
    public void fromFutureTimeout() throws Exception {
        Single.fromFuture(Flowable.never().toFuture(), 1, TimeUnit.SECONDS, Schedulers.io())
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TimeoutException.class);
    }

    @Test
    public void fromPublisher() {
        Single.fromPublisher(Flowable.just(1))
        .test()
        .assertResult(1);
    }
}
