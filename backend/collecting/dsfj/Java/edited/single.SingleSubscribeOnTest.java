

package io.reactivex.internal.operators.single;

import static org.junit.Assert.assertTrue;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.*;
import io.reactivex.subjects.PublishSubject;

public class SingleSubscribeOnTest {

    @Test
    public void normal() {
        List<Throwable> list = TestHelper.trackPluginErrors();
        try {
            TestScheduler scheduler = new TestScheduler();

            TestObserver<Integer> to = Single.just(1)
            .subscribeOn(scheduler)
            .test();

            scheduler.advanceTimeBy(1, TimeUnit.SECONDS);

            to.assertResult(1);

            assertTrue(list.toString(), list.isEmpty());
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishSubject.create().singleOrError().subscribeOn(new TestScheduler()));
    }

    @Test
    public void error() {
        Single.error(new TestException())
        .subscribeOn(Schedulers.single())
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TestException.class);
    }
}
