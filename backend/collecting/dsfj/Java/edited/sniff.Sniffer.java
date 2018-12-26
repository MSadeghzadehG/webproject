

package org.elasticsearch.client.sniff;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.HttpHost;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestClientBuilder;

import java.io.Closeable;
import java.io.IOException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;


public class Sniffer implements Closeable {

    private static final Log logger = LogFactory.getLog(Sniffer.class);
    private static final String SNIFFER_THREAD_NAME = "es_rest_client_sniffer";

    private final Task task;

    Sniffer(RestClient restClient, HostsSniffer hostsSniffer, long sniffInterval, long sniffAfterFailureDelay) {
        this.task = new Task(hostsSniffer, restClient, sniffInterval, sniffAfterFailureDelay);
    }

    
    public void sniffOnFailure(HttpHost failedHost) {
        this.task.sniffOnFailure(failedHost);
    }

    @Override
    public void close() throws IOException {
        task.shutdown();
    }

    private static class Task implements Runnable {
        private final HostsSniffer hostsSniffer;
        private final RestClient restClient;

        private final long sniffIntervalMillis;
        private final long sniffAfterFailureDelayMillis;
        private final ScheduledExecutorService scheduledExecutorService;
        private final AtomicBoolean running = new AtomicBoolean(false);
        private ScheduledFuture<?> scheduledFuture;

        private Task(HostsSniffer hostsSniffer, RestClient restClient, long sniffIntervalMillis, long sniffAfterFailureDelayMillis) {
            this.hostsSniffer = hostsSniffer;
            this.restClient = restClient;
            this.sniffIntervalMillis = sniffIntervalMillis;
            this.sniffAfterFailureDelayMillis = sniffAfterFailureDelayMillis;
            SnifferThreadFactory threadFactory = new SnifferThreadFactory(SNIFFER_THREAD_NAME);
            this.scheduledExecutorService = Executors.newScheduledThreadPool(1, threadFactory);
            scheduleNextRun(0);
        }

        synchronized void scheduleNextRun(long delayMillis) {
            if (scheduledExecutorService.isShutdown() == false) {
                try {
                    if (scheduledFuture != null) {
                                                this.scheduledFuture.cancel(false);
                    }
                    logger.debug("scheduling next sniff in " + delayMillis + " ms");
                    this.scheduledFuture = this.scheduledExecutorService.schedule(this, delayMillis, TimeUnit.MILLISECONDS);
                } catch(Exception e) {
                    logger.error("error while scheduling next sniffer task", e);
                }
            }
        }

        @Override
        public void run() {
            sniff(null, sniffIntervalMillis);
        }

        void sniffOnFailure(HttpHost failedHost) {
            sniff(failedHost, sniffAfterFailureDelayMillis);
        }

        void sniff(HttpHost excludeHost, long nextSniffDelayMillis) {
            if (running.compareAndSet(false, true)) {
                try {
                    List<HttpHost> sniffedHosts = hostsSniffer.sniffHosts();
                    logger.debug("sniffed hosts: " + sniffedHosts);
                    if (excludeHost != null) {
                        sniffedHosts.remove(excludeHost);
                    }
                    if (sniffedHosts.isEmpty()) {
                        logger.warn("no hosts to set, hosts will be updated at the next sniffing round");
                    } else {
                        this.restClient.setHosts(sniffedHosts.toArray(new HttpHost[sniffedHosts.size()]));
                    }
                } catch (Exception e) {
                    logger.error("error while sniffing nodes", e);
                } finally {
                    scheduleNextRun(nextSniffDelayMillis);
                    running.set(false);
                }
            }
        }

        synchronized void shutdown() {
            scheduledExecutorService.shutdown();
            try {
                if (scheduledExecutorService.awaitTermination(1000, TimeUnit.MILLISECONDS)) {
                    return;
                }
                scheduledExecutorService.shutdownNow();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    
    public static SnifferBuilder builder(RestClient restClient) {
        return new SnifferBuilder(restClient);
    }

    private static class SnifferThreadFactory implements ThreadFactory {

        private final AtomicInteger threadNumber = new AtomicInteger(1);
        private final String namePrefix;
        private final ThreadFactory originalThreadFactory;

        private SnifferThreadFactory(String namePrefix) {
            this.namePrefix = namePrefix;
            this.originalThreadFactory = AccessController.doPrivileged(new PrivilegedAction<ThreadFactory>() {
                @Override
                public ThreadFactory run() {
                    return Executors.defaultThreadFactory();
                }
            });
        }

        @Override
        public Thread newThread(final Runnable r) {
            return AccessController.doPrivileged(new PrivilegedAction<Thread>() {
                @Override
                public Thread run() {
                    Thread t = originalThreadFactory.newThread(r);
                    t.setName(namePrefix + "[T#" + threadNumber.getAndIncrement() + "]");
                    t.setDaemon(true);
                    return t;
                }
            });
        }
    }
}
