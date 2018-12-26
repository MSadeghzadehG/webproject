

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Test;
import org.reactivestreams.Subscriber;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Function;
import io.reactivex.internal.subscriptions.BooleanSubscription;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.schedulers.TestScheduler;

public class MaybeDelaySubscriptionTest {

    @Test
    public void normal() {
        PublishProcessor<Object> pp = PublishProcessor.create();

        TestObserver<Integer> to = Maybe.just(1).delaySubscription(pp)
        .test();

        assertTrue(pp.hasSubscribers());

        to.assertEmpty();

        pp.onNext("one");

        assertFalse(pp.hasSubscribers());

        to.assertResult(1);
    }

    @Test
    public void timed() {
        Maybe.just(1).delaySubscription(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(1);
    }

    @Test
    public void timedEmpty() {
        Maybe.<Integer>empty().delaySubscription(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult();
    }

    @Test
    public void timedTestScheduler() {
        TestScheduler scheduler = new TestScheduler();

        TestObserver<Integer> to = Maybe.just(1)
        .delaySubscription(100, TimeUnit.MILLISECONDS, scheduler)
        .test();

        to.assertEmpty();

        scheduler.advanceTimeBy(99, TimeUnit.MILLISECONDS);

        to.assertEmpty();

        scheduler.advanceTimeBy(1, TimeUnit.MILLISECONDS);

        to.assertResult(1);
    }

    @Test
    public void otherError() {
        Maybe.just(1).delaySubscription(Flowable.error(new TestException()))
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void mainError() {
        Maybe.error(new TestException())
        .delaySubscription(Flowable.empty())
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void withPublisherDispose() {
        TestHelper.checkDisposed(Maybe.just(1).delaySubscription(Flowable.never()));
    }

    @Test
    public void withPublisherDoubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeMaybe(new Function<Maybe<Object>, MaybeSource<Object>>() {
            @Override
            public MaybeSource<Object> apply(Maybe<Object> m) throws Exception {
                return m.delaySubscription(Flowable.just(1));
            }
        });
    }

    @Test
    public void withPublisherCallAfterTerminalEvent() {
        List<Throwable> errors = TestHelper.trackPluginErrors();

        try {
            Flowable<Integer> f = new Flowable<Integer>() {
                @Override
                protected void subscribeActual(Subscriber<? super Integer> observer) {
                    observer.onSubscribe(new BooleanSubscription());
                    observer.onNext(1);
                    observer.onError(new TestException());
                    observer.onComplete();
                    observer.onNext(2);
                }
            };

            Maybe.just(1).delaySubscription(f)
            .test()
            .assertResult(1);

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
