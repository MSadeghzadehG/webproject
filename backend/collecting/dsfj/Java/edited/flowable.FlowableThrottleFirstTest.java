

package io.reactivex.internal.operators.flowable;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.*;
import org.mockito.InOrder;
import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.exceptions.*;
import io.reactivex.internal.subscriptions.BooleanSubscription;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.schedulers.TestScheduler;

public class FlowableThrottleFirstTest {

    private TestScheduler scheduler;
    private Scheduler.Worker innerScheduler;
    private Subscriber<String> observer;

    @Before
    public void before() {
        scheduler = new TestScheduler();
        innerScheduler = scheduler.createWorker();
        observer = TestHelper.mockSubscriber();
    }

    @Test
    public void testThrottlingWithCompleted() {
        Flowable<String> source = Flowable.unsafeCreate(new Publisher<String>() {
            @Override
            public void subscribe(Subscriber<? super String> observer) {
                observer.onSubscribe(new BooleanSubscription());
                publishNext(observer, 100, "one");                    publishNext(observer, 300, "two");                    publishNext(observer, 900, "three");                   publishNext(observer, 905, "four");                   publishCompleted(observer, 1000);                 }
        });

        Flowable<String> sampled = source.throttleFirst(400, TimeUnit.MILLISECONDS, scheduler);
        sampled.subscribe(observer);

        InOrder inOrder = inOrder(observer);

        scheduler.advanceTimeTo(1000, TimeUnit.MILLISECONDS);
        inOrder.verify(observer, times(1)).onNext("one");
        inOrder.verify(observer, times(0)).onNext("two");
        inOrder.verify(observer, times(1)).onNext("three");
        inOrder.verify(observer, times(0)).onNext("four");
        inOrder.verify(observer, times(1)).onComplete();
        inOrder.verifyNoMoreInteractions();
    }

    @Test
    public void testThrottlingWithError() {
        Flowable<String> source = Flowable.unsafeCreate(new Publisher<String>() {
            @Override
            public void subscribe(Subscriber<? super String> observer) {
                observer.onSubscribe(new BooleanSubscription());
                Exception error = new TestException();
                publishNext(observer, 100, "one");                    publishNext(observer, 200, "two");                    publishError(observer, 300, error);               }
        });

        Flowable<String> sampled = source.throttleFirst(400, TimeUnit.MILLISECONDS, scheduler);
        sampled.subscribe(observer);

        InOrder inOrder = inOrder(observer);

        scheduler.advanceTimeTo(400, TimeUnit.MILLISECONDS);
        inOrder.verify(observer).onNext("one");
        inOrder.verify(observer).onError(any(TestException.class));
        inOrder.verifyNoMoreInteractions();
    }

    private <T> void publishCompleted(final Subscriber<T> observer, long delay) {
        innerScheduler.schedule(new Runnable() {
            @Override
            public void run() {
                observer.onComplete();
            }
        }, delay, TimeUnit.MILLISECONDS);
    }

    private <T> void publishError(final Subscriber<T> observer, long delay, final Exception error) {
        innerScheduler.schedule(new Runnable() {
            @Override
            public void run() {
                observer.onError(error);
            }
        }, delay, TimeUnit.MILLISECONDS);
    }

    private <T> void publishNext(final Subscriber<T> observer, long delay, final T value) {
        innerScheduler.schedule(new Runnable() {
            @Override
            public void run() {
                observer.onNext(value);
            }
        }, delay, TimeUnit.MILLISECONDS);
    }

    @Test
    public void testThrottle() {
        Subscriber<Integer> observer = TestHelper.mockSubscriber();
        TestScheduler s = new TestScheduler();
        PublishProcessor<Integer> o = PublishProcessor.create();
        o.throttleFirst(500, TimeUnit.MILLISECONDS, s).subscribe(observer);

                s.advanceTimeTo(0, TimeUnit.MILLISECONDS);
        o.onNext(1);         o.onNext(2);         s.advanceTimeTo(501, TimeUnit.MILLISECONDS);
        o.onNext(3);         s.advanceTimeTo(600, TimeUnit.MILLISECONDS);
        o.onNext(4);         s.advanceTimeTo(700, TimeUnit.MILLISECONDS);
        o.onNext(5);         o.onNext(6);         s.advanceTimeTo(1001, TimeUnit.MILLISECONDS);
        o.onNext(7);         s.advanceTimeTo(1501, TimeUnit.MILLISECONDS);
        o.onComplete();

        InOrder inOrder = inOrder(observer);
        inOrder.verify(observer).onNext(1);
        inOrder.verify(observer).onNext(3);
        inOrder.verify(observer).onNext(7);
        inOrder.verify(observer).onComplete();
        inOrder.verifyNoMoreInteractions();
    }


    @Test
    public void throttleFirstDefaultScheduler() {
        Flowable.just(1).throttleFirst(100, TimeUnit.MILLISECONDS)
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(1);
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Flowable.just(1).throttleFirst(1, TimeUnit.DAYS));
    }

    @Test
    public void badSource() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            new Flowable<Integer>() {
                @Override
                protected void subscribeActual(Subscriber<? super Integer> observer) {
                    observer.onSubscribe(new BooleanSubscription());
                    observer.onNext(1);
                    observer.onNext(2);
                    observer.onComplete();
                    observer.onNext(3);
                    observer.onError(new TestException());
                    observer.onComplete();
                }
            }
            .throttleFirst(1, TimeUnit.DAYS)
            .test()
            .assertResult(1);

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void backpressureNoRequest() {
        Flowable.range(1, 3)
        .throttleFirst(1, TimeUnit.MINUTES)
        .test(0L)
        .assertFailure(MissingBackpressureException.class);
    }
}
