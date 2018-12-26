

package org.elasticsearch.common.util.concurrent;

import org.elasticsearch.test.ESTestCase;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.atomic.AtomicInteger;

import static org.hamcrest.Matchers.equalTo;

public class SizeBlockingQueueTests extends ESTestCase {

    
    public void testQueueSize() throws InterruptedException {
        final int capacity = randomIntBetween(1, 32);
        final BlockingQueue<Integer> blockingQueue = new ArrayBlockingQueue<>(capacity);
        final SizeBlockingQueue<Integer> sizeBlockingQueue = new SizeBlockingQueue<>(blockingQueue, capacity);

                for (int i = 0; i < capacity; i++) {
            sizeBlockingQueue.offer(i);
        }


        final int iterations = 1 << 16;
        final CyclicBarrier barrier = new CyclicBarrier(2);

                final Thread queueOfferThread = new Thread(() -> {
            for (int i = 0; i < iterations; i++) {
                try {
                                        barrier.await();
                } catch (final BrokenBarrierException | InterruptedException e) {
                    throw new RuntimeException(e);
                }
                sizeBlockingQueue.offer(capacity + i);
            }
        });
        queueOfferThread.start();

                final AtomicInteger maxSize = new AtomicInteger();
        final Thread queueSizeThread = new Thread(() -> {
            for (int i = 0; i < iterations; i++) {
                try {
                                        barrier.await();
                } catch (final BrokenBarrierException | InterruptedException e) {
                    throw new RuntimeException(e);
                }
                maxSize.set(Math.max(maxSize.get(), sizeBlockingQueue.size()));
            }
        });
        queueSizeThread.start();

                queueOfferThread.join();
        queueSizeThread.join();

                assertThat(maxSize.get(), equalTo(capacity));
    }

}
