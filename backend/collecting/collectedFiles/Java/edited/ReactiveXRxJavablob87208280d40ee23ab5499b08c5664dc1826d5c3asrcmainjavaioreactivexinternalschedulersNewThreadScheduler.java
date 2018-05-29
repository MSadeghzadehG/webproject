

package io.reactivex.internal.schedulers;

import io.reactivex.Scheduler;
import io.reactivex.annotations.NonNull;

import java.util.concurrent.ThreadFactory;


public final class NewThreadScheduler extends Scheduler {

    final ThreadFactory threadFactory;

    private static final String THREAD_NAME_PREFIX = "RxNewThreadScheduler";
    private static final RxThreadFactory THREAD_FACTORY;

    
    private static final String KEY_NEWTHREAD_PRIORITY = "rx2.newthread-priority";

    static {
        int priority = Math.max(Thread.MIN_PRIORITY, Math.min(Thread.MAX_PRIORITY,
                Integer.getInteger(KEY_NEWTHREAD_PRIORITY, Thread.NORM_PRIORITY)));

        THREAD_FACTORY = new RxThreadFactory(THREAD_NAME_PREFIX, priority);
    }

    public NewThreadScheduler() {
        this(THREAD_FACTORY);
    }

    public NewThreadScheduler(ThreadFactory threadFactory) {
        this.threadFactory = threadFactory;
    }

    @NonNull
    @Override
    public Worker createWorker() {
        return new NewThreadWorker(threadFactory);
    }
}
