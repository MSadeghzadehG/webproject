

package io.reactivex.internal.schedulers;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;


public final class SchedulerPoolFactory {
    
    private SchedulerPoolFactory() {
        throw new IllegalStateException("No instances!");
    }

    static final String PURGE_ENABLED_KEY = "rx2.purge-enabled";

    
    public static final boolean PURGE_ENABLED;

    static final String PURGE_PERIOD_SECONDS_KEY = "rx2.purge-period-seconds";

    
    public static final int PURGE_PERIOD_SECONDS;

    static final AtomicReference<ScheduledExecutorService> PURGE_THREAD =
            new AtomicReference<ScheduledExecutorService>();

            static final Map<ScheduledThreadPoolExecutor, Object> POOLS =
            new ConcurrentHashMap<ScheduledThreadPoolExecutor, Object>();

    
    public static void start() {
        tryStart(PURGE_ENABLED);
    }

    static void tryStart(boolean purgeEnabled) {
        if (purgeEnabled) {
            for (;;) {
                ScheduledExecutorService curr = PURGE_THREAD.get();
                if (curr != null) {
                    return;
                }
                ScheduledExecutorService next = Executors.newScheduledThreadPool(1, new RxThreadFactory("RxSchedulerPurge"));
                if (PURGE_THREAD.compareAndSet(curr, next)) {

                    next.scheduleAtFixedRate(new ScheduledTask(), PURGE_PERIOD_SECONDS, PURGE_PERIOD_SECONDS, TimeUnit.SECONDS);

                    return;
                } else {
                    next.shutdownNow();
                }
            }
        }
    }

    
    public static void shutdown() {
        ScheduledExecutorService exec = PURGE_THREAD.getAndSet(null);
        if (exec != null) {
            exec.shutdownNow();
        }
        POOLS.clear();
    }

    static {
        Properties properties = System.getProperties();

        PurgeProperties pp = new PurgeProperties();
        pp.load(properties);

        PURGE_ENABLED = pp.purgeEnable;
        PURGE_PERIOD_SECONDS = pp.purgePeriod;

        start();
    }

    static final class PurgeProperties {

        boolean purgeEnable;

        int purgePeriod;

        void load(Properties properties) {
            if (properties.containsKey(PURGE_ENABLED_KEY)) {
                purgeEnable = Boolean.parseBoolean(properties.getProperty(PURGE_ENABLED_KEY));
            } else {
                purgeEnable = true;
            }

            if (purgeEnable && properties.containsKey(PURGE_PERIOD_SECONDS_KEY)) {
                try {
                    purgePeriod = Integer.parseInt(properties.getProperty(PURGE_PERIOD_SECONDS_KEY));
                } catch (NumberFormatException ex) {
                    purgePeriod = 1;
                }
            } else {
                purgePeriod = 1;
            }
        }
    }

    
    public static ScheduledExecutorService create(ThreadFactory factory) {
        final ScheduledExecutorService exec = Executors.newScheduledThreadPool(1, factory);
        tryPutIntoPool(PURGE_ENABLED, exec);
        return exec;
    }

    static void tryPutIntoPool(boolean purgeEnabled, ScheduledExecutorService exec) {
        if (purgeEnabled && exec instanceof ScheduledThreadPoolExecutor) {
            ScheduledThreadPoolExecutor e = (ScheduledThreadPoolExecutor) exec;
            POOLS.put(e, exec);
        }
    }

    static final class ScheduledTask implements Runnable {
        @Override
        public void run() {
            for (ScheduledThreadPoolExecutor e : new ArrayList<ScheduledThreadPoolExecutor>(POOLS.keySet())) {
                if (e.isShutdown()) {
                    POOLS.remove(e);
                } else {
                    e.purge();
                }
            }
        }
    }
}
