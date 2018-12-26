

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.assertTrue;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.*;
import io.reactivex.subjects.PublishSubject;

public class CompletableSubscribeOnTest {

    @Test
    public void normal() {
        List<Throwable> list = TestHelper.trackPluginErrors();
        try {
            TestScheduler scheduler = new TestScheduler();

            TestObserver<Void> to = Completable.complete()
            .subscribeOn(scheduler)
            .test();

            scheduler.advanceTimeBy(1, TimeUnit.SECONDS);

            to.assertResult();

            assertTrue(list.toString(), list.isEmpty());
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishSubject.create().ignoreElements().subscribeOn(new TestScheduler()));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeCompletable(new Function<Completable, CompletableSource>() {
            @Override
            public CompletableSource apply(Completable c) throws Exception {
                return c.subscribeOn(Schedulers.single());
            }
        });
    }
}
