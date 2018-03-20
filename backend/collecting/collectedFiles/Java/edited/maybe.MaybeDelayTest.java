

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Function;
import io.reactivex.observers.TestObserver;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.schedulers.TestScheduler;

public class MaybeDelayTest {

    @Test
    public void success() {
        Maybe.just(1).delay(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(1);
    }

    @Test
    public void error() {
        Maybe.error(new TestException()).delay(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TestException.class);
    }

    @Test
    public void complete() {
        Maybe.empty().delay(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult();
    }

    @Test(expected = NullPointerException.class)
    public void nullUnit() {
        Maybe.just(1).delay(1, null);
    }

    @Test(expected = NullPointerException.class)
    public void nullScheduler() {
        Maybe.just(1).delay(1, TimeUnit.MILLISECONDS, null);
    }

    @Test
    public void disposeDuringDelay() {
        TestScheduler scheduler = new TestScheduler();

        TestObserver<Integer> to = Maybe.just(1).delay(100, TimeUnit.MILLISECONDS, scheduler)
        .test();

        to.cancel();

        scheduler.advanceTimeBy(1, TimeUnit.SECONDS);

        to.assertEmpty();
    }

    @Test
    public void dispose() {
        PublishProcessor<Integer> pp = PublishProcessor.create();

        TestObserver<Integer> to = pp.singleElement().delay(100, TimeUnit.MILLISECONDS).test();

        assertTrue(pp.hasSubscribers());

        to.cancel();

        assertFalse(pp.hasSubscribers());
    }

    @Test
    public void isDisposed() {
        PublishProcessor<Integer> pp = PublishProcessor.create();

        TestHelper.checkDisposed(pp.singleElement().delay(100, TimeUnit.MILLISECONDS));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeMaybe(new Function<Maybe<Object>, Maybe<Object>>() {
            @Override
            public Maybe<Object> apply(Maybe<Object> f) throws Exception {
                return f.delay(100, TimeUnit.MILLISECONDS);
            }
        });
    }
}
