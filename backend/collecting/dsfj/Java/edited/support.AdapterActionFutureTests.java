

package org.elasticsearch.action.support;

import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.test.ESTestCase;

import java.util.Objects;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

public class AdapterActionFutureTests extends ESTestCase {

    public void testInterruption() throws Exception {
        final AdapterActionFuture<String, Integer> adapter =
                new AdapterActionFuture<String, Integer>() {
                    @Override
                    protected String convert(final Integer listenerResponse) {
                        return Objects.toString(listenerResponse);
                    }
                };

                final Runnable runnable = () -> {
            final int method = randomIntBetween(0, 4);
            switch (method) {
                case 0:
                    adapter.actionGet();
                    break;
                case 1:
                    adapter.actionGet("30s");
                    break;
                case 2:
                    adapter.actionGet(30000);
                    break;
                case 3:
                    adapter.actionGet(TimeValue.timeValueSeconds(30));
                    break;
                case 4:
                    adapter.actionGet(30, TimeUnit.SECONDS);
                    break;
                default:
                    throw new AssertionError(method);
            }
        };

        final CyclicBarrier barrier = new CyclicBarrier(2);
        final Thread main = Thread.currentThread();
        final Thread thread = new Thread(() -> {
            try {
                barrier.await();
            } catch (final BrokenBarrierException | InterruptedException e) {
                throw new RuntimeException(e);
            }
            main.interrupt();
        });
        thread.start();

        final AtomicBoolean interrupted = new AtomicBoolean();

        barrier.await();

        try {
            runnable.run();
        } catch (final IllegalStateException e) {
            interrupted.set(Thread.interrupted());
        }
                assertTrue(interrupted.get());

        thread.join();
    }

}
