

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.*;

import io.reactivex.*;
import io.reactivex.functions.Action;
import io.reactivex.subscribers.TestSubscriber;

public class FlowableToSingleTest {

    @Test
    public void testJustSingleItemObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Single<String> single = Flowable.just("Hello World!").single("");
        single.toFlowable().subscribe(subscriber);

        subscriber.assertResult("Hello World!");
    }

    @Test
    public void testErrorObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        IllegalArgumentException error = new IllegalArgumentException("Error");
        Single<String> single = Flowable.<String>error(error).single("");
        single.toFlowable().subscribe(subscriber);

        subscriber.assertError(error);
    }

    @Test
    public void testJustTwoEmissionsObservableThrowsError() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Single<String> single = Flowable.just("First", "Second").single("");
        single.toFlowable().subscribe(subscriber);

        subscriber.assertError(IllegalArgumentException.class);
    }

    @Test
    public void testEmptyObservable() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Single<String> single = Flowable.<String>empty().single("");
        single.toFlowable().subscribe(subscriber);

        subscriber.assertResult("");
    }

    @Test
    public void testRepeatObservableThrowsError() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        Single<String> single = Flowable.just("First", "Second").repeat().single("");
        single.toFlowable().subscribe(subscriber);

        subscriber.assertError(IllegalArgumentException.class);
    }

    @Test
    public void testShouldUseUnsafeSubscribeInternallyNotSubscribe() {
        TestSubscriber<String> subscriber = TestSubscriber.create();
        final AtomicBoolean unsubscribed = new AtomicBoolean(false);
        Single<String> single = Flowable.just("Hello World!").doOnCancel(new Action() {

            @Override
            public void run() {
                unsubscribed.set(true);
            }}).single("");
        single.toFlowable().subscribe(subscriber);
        subscriber.assertComplete();
        Assert.assertFalse(unsubscribed.get());
    }
}
