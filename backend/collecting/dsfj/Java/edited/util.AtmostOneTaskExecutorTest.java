package jenkins.util;

import hudson.util.OneShotEvent;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicInteger;
import static org.junit.Assert.*;
import org.junit.Test;

public class AtmostOneTaskExecutorTest {

    @SuppressWarnings("empty-statement")
    @Test
    public void doubleBooking() throws Exception {
        AtomicInteger counter = new AtomicInteger();
        OneShotEvent lock = new OneShotEvent();
        Future<?> f1, f2;

        ExecutorService base = Executors.newCachedThreadPool();
        AtmostOneTaskExecutor<?> es = new AtmostOneTaskExecutor<Void>(base, () -> {
            counter.incrementAndGet();
            try {
                lock.block();
            } catch (InterruptedException x) {
                assert false : x;
            }
            return null;
        });
        f1 = es.submit();
        while (counter.get() == 0) {
                    }

        f2 = es.submit();         Thread.sleep(500);           assertEquals(1, counter.get());
        assertFalse(f2.isDone());

        lock.signal(); 
        f1.get();   
                f2.get();
        assertEquals(2, counter.get());
    }

}
