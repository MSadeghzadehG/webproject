

package io.reactivex.observable;

import static org.junit.Assert.assertNotNull;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.observers.DefaultObserver;
import io.reactivex.schedulers.Schedulers;

public class ObservableErrorHandlingTests {

    
    @Test
    public void testOnNextError() throws InterruptedException {
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Throwable> caughtError = new AtomicReference<Throwable>();
        Observable<Long> o = Observable.interval(50, TimeUnit.MILLISECONDS);
        Observer<Long> observer = new DefaultObserver<Long>() {

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
        Observable<Long> o = Observable.interval(50, TimeUnit.MILLISECONDS);
        Observer<Long> observer = new DefaultObserver<Long>() {

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
