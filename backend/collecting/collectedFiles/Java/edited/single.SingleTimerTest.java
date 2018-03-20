

package io.reactivex.single;

import org.junit.Test;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import io.reactivex.Single;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.TestScheduler;

import static org.junit.Assert.assertEquals;

public class SingleTimerTest {
    @Test
    public void timer() {
        final TestScheduler testScheduler = new TestScheduler();

        final AtomicLong atomicLong = new AtomicLong();
        Single.timer(2, TimeUnit.SECONDS, testScheduler).subscribe(new Consumer<Long>() {
            @Override
            public void accept(final Long value) throws Exception {
                atomicLong.incrementAndGet();
            }
        });

        assertEquals(0, atomicLong.get());

        testScheduler.advanceTimeBy(1, TimeUnit.SECONDS);

        assertEquals(0, atomicLong.get());

        testScheduler.advanceTimeBy(1, TimeUnit.SECONDS);

        assertEquals(1, atomicLong.get());
    }
}
