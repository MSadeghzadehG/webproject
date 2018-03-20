

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.*;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Action;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.TestScheduler;
import io.reactivex.subjects.PublishSubject;

public class CompletableDisposeOnTest {

    @Test
    public void cancelDelayed() {
        TestScheduler scheduler = new TestScheduler();

        PublishSubject<Integer> ps = PublishSubject.create();

        ps.ignoreElements()
        .unsubscribeOn(scheduler)
        .test()
        .cancel();

        assertTrue(ps.hasObservers());

        scheduler.advanceTimeBy(1, TimeUnit.SECONDS);

        assertFalse(ps.hasObservers());
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishSubject.create().ignoreElements().unsubscribeOn(new TestScheduler()));
    }

    @Test
    public void completeAfterCancel() {
        TestScheduler scheduler = new TestScheduler();

        PublishSubject<Integer> ps = PublishSubject.create();

        TestObserver<Void> to = ps.ignoreElements()
        .unsubscribeOn(scheduler)
        .test();

        to.dispose();

        ps.onComplete();

        to.assertEmpty();
    }

    @Test
    public void errorAfterCancel() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            TestScheduler scheduler = new TestScheduler();

            PublishSubject<Integer> ps = PublishSubject.create();

            TestObserver<Void> to = ps.ignoreElements()
            .unsubscribeOn(scheduler)
            .test();

            to.dispose();

            ps.onError(new TestException());

            to.assertEmpty();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void normal() {
        TestScheduler scheduler = new TestScheduler();

        final int[] call = { 0 };

        Completable.complete()
        .doOnDispose(new Action() {
            @Override
            public void run() throws Exception {
                call[0]++;
            }
        })
        .unsubscribeOn(scheduler)
        .test()
        .assertResult();

        scheduler.triggerActions();

        assertEquals(0, call[0]);
    }

    @Test
    public void error() {
        TestScheduler scheduler = new TestScheduler();

        final int[] call = { 0 };

        Completable.error(new TestException())
        .doOnDispose(new Action() {
            @Override
            public void run() throws Exception {
                call[0]++;
            }
        })
        .unsubscribeOn(scheduler)
        .test()
        .assertFailure(TestException.class);

        scheduler.triggerActions();

        assertEquals(0, call[0]);
    }
}
