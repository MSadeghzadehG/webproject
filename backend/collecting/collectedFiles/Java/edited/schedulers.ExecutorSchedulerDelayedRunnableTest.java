

package io.reactivex.internal.schedulers;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.exceptions.TestException;
import io.reactivex.internal.schedulers.ExecutorScheduler.DelayedRunnable;

public class ExecutorSchedulerDelayedRunnableTest {


    @Test(expected = TestException.class)
    public void delayedRunnableCrash() {
        DelayedRunnable dl = new DelayedRunnable(new Runnable() {
            @Override
            public void run() {
                throw new TestException();
            }
        });
        dl.run();
    }

    @Test
    public void dispose() {
        final AtomicInteger count = new AtomicInteger();
        DelayedRunnable dl = new DelayedRunnable(new Runnable() {
            @Override
            public void run() {
                count.incrementAndGet();
            }
        });

        dl.dispose();
        dl.dispose();

        dl.run();

        assertEquals(0, count.get());
    }
}
