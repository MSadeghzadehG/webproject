

package io.reactivex.internal.operators.completable;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.functions.Consumer;
import org.junit.Test;

import io.reactivex.Completable;
import io.reactivex.schedulers.Schedulers;

import static org.junit.Assert.assertNotEquals;

public class CompletableDelayTest {

    @Test
    public void delayCustomScheduler() {

        Completable.complete()
        .delay(100, TimeUnit.MILLISECONDS, Schedulers.trampoline())
        .test()
        .assertResult();
    }

    @Test
    public void testOnErrorCalledOnScheduler() throws Exception {
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Thread> thread = new AtomicReference<Thread>();

        Completable.error(new Exception())
                .delay(0, TimeUnit.MILLISECONDS, Schedulers.newThread())
                .doOnError(new Consumer<Throwable>() {
                    @Override
                    public void accept(Throwable throwable) throws Exception {
                        thread.set(Thread.currentThread());
                        latch.countDown();
                    }
                })
                .onErrorComplete()
                .subscribe();

        latch.await();

        assertNotEquals(Thread.currentThread(), thread.get());
    }

}
