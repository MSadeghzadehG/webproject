

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.assertTrue;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.*;

public class MaybeTimerTest {

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Maybe.timer(1, TimeUnit.SECONDS, new TestScheduler()));
    }

    @Test
    public void timerInterruptible() throws Exception {
        ScheduledExecutorService exec = Executors.newSingleThreadScheduledExecutor();
        try {
            for (Scheduler s : new Scheduler[] { Schedulers.single(), Schedulers.computation(), Schedulers.newThread(), Schedulers.io(), Schedulers.from(exec) }) {
                final AtomicBoolean interrupted = new AtomicBoolean();
                TestObserver<Long> to = Maybe.timer(1, TimeUnit.MILLISECONDS, s)
                .map(new Function<Long, Long>() {
                    @Override
                    public Long apply(Long v) throws Exception {
                        try {
                        Thread.sleep(3000);
                        } catch (InterruptedException ex) {
                            interrupted.set(true);
                        }
                        return v;
                    }
                })
                .test();

                Thread.sleep(500);

                to.cancel();

                Thread.sleep(500);

                assertTrue(s.getClass().getSimpleName(), interrupted.get());
            }
        } finally {
            exec.shutdown();
        }
    }

}
