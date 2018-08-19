

package io.reactivex.schedulers;

import static org.junit.Assert.*;

import java.util.concurrent.TimeUnit;

import org.junit.*;

import io.reactivex.*;
import io.reactivex.Scheduler.Worker;
import io.reactivex.disposables.*;
import io.reactivex.functions.*;
import io.reactivex.internal.schedulers.IoScheduler;

public class CachedThreadSchedulerTest extends AbstractSchedulerConcurrencyTests {

    @Override
    protected Scheduler getScheduler() {
        return Schedulers.io();
    }

    
    @Test
    public final void testIOScheduler() {

        Flowable<Integer> o1 = Flowable.just(1, 2, 3, 4, 5);
        Flowable<Integer> o2 = Flowable.just(6, 7, 8, 9, 10);
        Flowable<String> o = Flowable.merge(o1, o2).map(new Function<Integer, String>() {

            @Override
            public String apply(Integer t) {
                assertTrue(Thread.currentThread().getName().startsWith("RxCachedThreadScheduler"));
                return "Value_" + t + "_Thread_" + Thread.currentThread().getName();
            }
        });

        o.subscribeOn(Schedulers.io()).blockingForEach(new Consumer<String>() {

            @Override
            public void accept(String t) {
                System.out.println("t: " + t);
            }
        });
    }

    @Test
    @Ignore("Unhandled errors are no longer thrown")
    public final void testUnhandledErrorIsDeliveredToThreadHandler() throws InterruptedException {
        SchedulerTestHelper.testUnhandledErrorIsDeliveredToThreadHandler(getScheduler());
    }

    @Test
    public final void testHandledErrorIsNotDeliveredToThreadHandler() throws InterruptedException {
        SchedulerTestHelper.testHandledErrorIsNotDeliveredToThreadHandler(getScheduler());
    }

    @Test(timeout = 60000)
    public void testCancelledTaskRetention() throws InterruptedException {
        Worker w = Schedulers.io().createWorker();
        try {
            ExecutorSchedulerTest.testCancelledRetention(w, false);
        } finally {
            w.dispose();
        }
        w = Schedulers.io().createWorker();
        try {
            ExecutorSchedulerTest.testCancelledRetention(w, true);
        } finally {
            w.dispose();
        }
    }

    @Test
    public void workerDisposed() {
        Worker w = Schedulers.io().createWorker();

        assertFalse(((Disposable)w).isDisposed());

        w.dispose();

        assertTrue(((Disposable)w).isDisposed());
    }

    @Test
    public void shutdownRejects() {
        final int[] calls = { 0 };

        Runnable r = new Runnable() {
            @Override
            public void run() {
                calls[0]++;
            }
        };

        IoScheduler s = new IoScheduler();
        s.shutdown();
        s.shutdown();

        s.scheduleDirect(r);

        s.scheduleDirect(r, 1, TimeUnit.SECONDS);

        s.schedulePeriodicallyDirect(r, 1, 1, TimeUnit.SECONDS);

        Worker w = s.createWorker();
        w.dispose();

        assertEquals(Disposables.disposed(), w.schedule(r));

        assertEquals(Disposables.disposed(), w.schedule(r, 1, TimeUnit.SECONDS));

        assertEquals(Disposables.disposed(), w.schedulePeriodically(r, 1, 1, TimeUnit.SECONDS));

        assertEquals(0, calls[0]);
    }
}
