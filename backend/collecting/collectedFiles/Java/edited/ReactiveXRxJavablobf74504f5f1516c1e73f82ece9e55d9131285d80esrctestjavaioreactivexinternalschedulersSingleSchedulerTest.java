

package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;

import io.reactivex.schedulers.AbstractSchedulerTests;
import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.Scheduler.Worker;
import io.reactivex.disposables.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.schedulers.SingleScheduler.ScheduledWorker;
import io.reactivex.schedulers.Schedulers;

public class SingleSchedulerTest extends AbstractSchedulerTests {

    @Test
    public void shutdownRejects() {
        final int[] calls = { 0 };

        Runnable r = new Runnable() {
            @Override
            public void run() {
                calls[0]++;
            }
        };

        Scheduler s = new SingleScheduler();
        s.shutdown();

        assertEquals(Disposables.disposed(), s.scheduleDirect(r));

        assertEquals(Disposables.disposed(), s.scheduleDirect(r, 1, TimeUnit.SECONDS));

        assertEquals(Disposables.disposed(), s.schedulePeriodicallyDirect(r, 1, 1, TimeUnit.SECONDS));

        Worker w = s.createWorker();
        ((ScheduledWorker)w).executor.shutdownNow();

        assertEquals(Disposables.disposed(), w.schedule(r));

        assertEquals(Disposables.disposed(), w.schedule(r, 1, TimeUnit.SECONDS));

        assertEquals(Disposables.disposed(), w.schedulePeriodically(r, 1, 1, TimeUnit.SECONDS));

        assertEquals(0, calls[0]);

        w.dispose();

        assertTrue(w.isDisposed());
    }

    @Test
    public void startRace() {
        final Scheduler s = new SingleScheduler();
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {
            s.shutdown();

            Runnable r1 = new Runnable() {
                @Override
                public void run() {
                    s.start();
                }
            };

            TestHelper.race(r1, r1);
        }
    }

    @Test(timeout = 1000)
    public void runnableDisposedAsync() throws Exception {
        final Scheduler s = Schedulers.single();
        Disposable d = s.scheduleDirect(Functions.EMPTY_RUNNABLE);

        while (!d.isDisposed()) {
            Thread.sleep(1);
        }
    }

    @Test(timeout = 1000)
    public void runnableDisposedAsyncCrash() throws Exception {
        final Scheduler s = Schedulers.single();

        Disposable d = s.scheduleDirect(new Runnable() {
            @Override
            public void run() {
                throw new IllegalStateException();
            }
        });

        while (!d.isDisposed()) {
            Thread.sleep(1);
        }
    }

    @Test(timeout = 1000)
    public void runnableDisposedAsyncTimed() throws Exception {
        final Scheduler s = Schedulers.single();

        Disposable d = s.scheduleDirect(Functions.EMPTY_RUNNABLE, 1, TimeUnit.MILLISECONDS);

        while (!d.isDisposed()) {
            Thread.sleep(1);
        }
    }

    @Override protected Scheduler getScheduler() {
        return Schedulers.single();
    }

}
