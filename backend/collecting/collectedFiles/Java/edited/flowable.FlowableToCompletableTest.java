
package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.assertFalse;

import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Action;
import io.reactivex.subscribers.TestSubscriber;

public class FlowableToCompletableTest {

    @Test
    public void testJustSingleItemObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Completable cmp = Flowable.just("Hello World!").ignoreElements();
        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertNoValues();
        subscriber.assertComplete();
        subscriber.assertNoErrors();
    }

    @Test
    public void testErrorObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        IllegalArgumentException error = new IllegalArgumentException("Error");
        Completable cmp = Flowable.<String>error(error).ignoreElements();
        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertError(error);
        subscriber.assertNoValues();
    }

    @Test
    public void testJustTwoEmissionsObservableThrowsError() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Completable cmp = Flowable.just("First", "Second").ignoreElements();
        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertNoErrors();
        subscriber.assertNoValues();
    }

    @Test
    public void testEmptyObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Completable cmp = Flowable.<String>empty().ignoreElements();
        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertNoErrors();
        subscriber.assertNoValues();
        subscriber.assertComplete();
    }

    @Test
    public void testNeverObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Completable cmp = Flowable.<String>never().ignoreElements();
        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertNotTerminated();
        subscriber.assertNoValues();
    }

    @Test
    public void testShouldUseUnsafeSubscribeInternallyNotSubscribe() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        final AtomicBoolean unsubscribed = new AtomicBoolean(false);
        Completable cmp = Flowable.just("Hello World!").doOnCancel(new Action() {

            @Override
            public void run() {
                unsubscribed.set(true);
            }}).ignoreElements();

        cmp.<String>toFlowable().subscribe(subscriber);

        subscriber.assertComplete();

        assertFalse(unsubscribed.get());
    }
}
