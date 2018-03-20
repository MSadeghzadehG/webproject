

package io.reactivex.flowable;

import static org.junit.Assert.assertNotNull;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;

import org.junit.Test;
import org.reactivestreams.Subscriber;

import io.reactivex.Flowable;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subscribers.DefaultSubscriber;

public class FlowableErrorHandlingTests {

    
    @Test
    public void testOnNextError() throws InterruptedException {
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Throwable> caughtError = new AtomicReference<Throwable>();
        Flowable<Long> o = Flowable.interval(50, TimeUnit.MILLISECONDS);
        Subscriber<Long> observer = new DefaultSubscriber<Long>() {

            @Override
            public void onComplete() {
                System.out.println("completed");
                latch.countDown();
            }

            @Override
            public void onError(Throwable e) {
                System.out.println("error: " + e);
                caughtError.set(e);
                latch.countDown();
            }

            @Override
            public void onNext(Long args) {
                throw new RuntimeException("forced failure");
            }
        };
        o.safeSubscribe(observer);

        latch.await(2000, TimeUnit.MILLISECONDS);
        assertNotNull(caughtError.get());
    }

    
    @Test
    public void testOnNextErrorAcrossThread() throws InterruptedException {
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Throwable> caughtError = new AtomicReference<Throwable>();
        Flowable<Long> o = Flowable.interval(50, TimeUnit.MILLISECONDS);
        Subscriber<Long> observer = new DefaultSubscriber<Long>() {

            @Override
            public void onComplete() {
                System.out.println("completed");
                latch.countDown();
            }

            @Override
            public void onError(Throwable e) {
                System.out.println("error: " + e);
                caughtError.set(e);
                latch.countDown();
            }

            @Override
            public void onNext(Long args) {
                throw new RuntimeException("forced failure");
            }
        };
        o.observeOn(Schedulers.newThread())
        .safeSubscribe(observer);

        latch.await(2000, TimeUnit.MILLISECONDS);
        assertNotNull(caughtError.get());
    }
}
