

package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.Scheduler.Worker;
import io.reactivex.internal.functions.Functions;

public class ImmediateThinSchedulerTest {

    @Test
    public void scheduleDirect() {
        final int[] count = { 0 };

        ImmediateThinScheduler.INSTANCE.scheduleDirect(new Runnable() {
            @Override
            public void run() {
                count[0]++;
            }
        });

        assertEquals(1, count[0]);
    }

    @Test(expected = UnsupportedOperationException.class)
    public void scheduleDirectTimed() {
        ImmediateThinScheduler.INSTANCE.scheduleDirect(Functions.EMPTY_RUNNABLE, 1, TimeUnit.SECONDS);
    }

    @Test(expected = UnsupportedOperationException.class)
    public void scheduleDirectPeriodic() {
        ImmediateThinScheduler.INSTANCE.schedulePeriodicallyDirect(Functions.EMPTY_RUNNABLE, 1, 1, TimeUnit.SECONDS);
    }
    @Test
    public void schedule() {
        final int[] count = { 0 };

        Worker w = ImmediateThinScheduler.INSTANCE.createWorker();

        assertFalse(w.isDisposed());

        w.schedule(new Runnable() {
            @Override
            public void run() {
                count[0]++;
            }
        });

        assertEquals(1, count[0]);
    }

    @Test(expected = UnsupportedOperationException.class)
    public void scheduleTimed() {
        ImmediateThinScheduler.INSTANCE.createWorker().schedule(Functions.EMPTY_RUNNABLE, 1, TimeUnit.SECONDS);
    }

    @Test(expected = UnsupportedOperationException.class)
    public void schedulePeriodic() {
        ImmediateThinScheduler.INSTANCE.createWorker().schedulePeriodically(Functions.EMPTY_RUNNABLE, 1, 1, TimeUnit.SECONDS);
    }
}
