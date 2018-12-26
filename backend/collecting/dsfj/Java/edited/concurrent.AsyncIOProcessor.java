
package org.elasticsearch.common.util.concurrent;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.common.collect.Tuple;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.Semaphore;
import java.util.function.Consumer;


public abstract class AsyncIOProcessor<Item> {
    private final Logger logger;
    private final ArrayBlockingQueue<Tuple<Item, Consumer<Exception>>> queue;
    private final Semaphore promiseSemaphore = new Semaphore(1);

    protected AsyncIOProcessor(Logger logger, int queueSize) {
        this.logger = logger;
        this.queue = new ArrayBlockingQueue<>(queueSize);
    }

    
    public final void put(Item item, Consumer<Exception> listener) {
        Objects.requireNonNull(item, "item must not be null");
        Objects.requireNonNull(listener, "listener must not be null");
                        
                final boolean promised = promiseSemaphore.tryAcquire();
        final Tuple<Item, Consumer<Exception>> itemTuple = new Tuple<>(item, listener);
        if (promised == false) {
                        try {
                queue.put(new Tuple<>(item, listener));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                listener.accept(e);
            }
        }

                        if (promised || promiseSemaphore.tryAcquire()) {
            final List<Tuple<Item, Consumer<Exception>>> candidates = new ArrayList<>();
            try {
                if (promised) {
                                        candidates.add(itemTuple);
                }
                                drainAndProcess(candidates);
            } finally {
                promiseSemaphore.release();             }
            while (queue.isEmpty() == false && promiseSemaphore.tryAcquire()) {
                                try {
                    drainAndProcess(candidates);
                } finally {
                    promiseSemaphore.release();
                }
            }
        }
    }

    private void drainAndProcess(List<Tuple<Item, Consumer<Exception>>> candidates) {
        queue.drainTo(candidates);
        processList(candidates);
        candidates.clear();
    }

    private void processList(List<Tuple<Item, Consumer<Exception>>> candidates) {
        Exception exception = null;
        if (candidates.isEmpty() == false) {
            try {
                write(candidates);
            } catch (Exception ex) {                 logger.debug("failed to write candidates", ex);
                                exception = ex;
            }
        }
        for (Tuple<Item, Consumer<Exception>> tuple : candidates) {
            Consumer<Exception> consumer = tuple.v2();
            try {
                consumer.accept(exception);
            } catch (Exception ex) {
                logger.warn("failed to notify callback", ex);
            }
        }
    }

    
    protected abstract void write(List<Tuple<Item, Consumer<Exception>>> candidates) throws IOException;
}
