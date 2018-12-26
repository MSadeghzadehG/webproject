

package org.elasticsearch.common.util.concurrent;

import org.elasticsearch.common.metrics.CounterMetric;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;

public class EsAbortPolicy implements XRejectedExecutionHandler {
    private final CounterMetric rejected = new CounterMetric();

    @Override
    public void rejectedExecution(Runnable r, ThreadPoolExecutor executor) {
        if (r instanceof AbstractRunnable) {
            if (((AbstractRunnable) r).isForceExecution()) {
                BlockingQueue<Runnable> queue = executor.getQueue();
                if (!(queue instanceof SizeBlockingQueue)) {
                    throw new IllegalStateException("forced execution, but expected a size queue");
                }
                try {
                    ((SizeBlockingQueue) queue).forcePut(r);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new IllegalStateException("forced execution, but got interrupted", e);
                }
                return;
            }
        }
        rejected.inc();
        throw new EsRejectedExecutionException("rejected execution of " + r + " on " + executor, executor.isShutdown());
    }

    @Override
    public long rejected() {
        return rejected.count();
    }
}
