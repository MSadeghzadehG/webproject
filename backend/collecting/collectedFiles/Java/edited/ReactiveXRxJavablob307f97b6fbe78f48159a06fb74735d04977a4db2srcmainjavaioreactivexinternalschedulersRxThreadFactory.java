

package io.reactivex.internal.schedulers;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicLong;


public final class RxThreadFactory extends AtomicLong implements ThreadFactory {

    private static final long serialVersionUID = -7789753024099756196L;

    final String prefix;

    final int priority;

    final boolean nonBlocking;


    public RxThreadFactory(String prefix) {
        this(prefix, Thread.NORM_PRIORITY, false);
    }

    public RxThreadFactory(String prefix, int priority) {
        this(prefix, priority, false);
    }

    public RxThreadFactory(String prefix, int priority, boolean nonBlocking) {
        this.prefix = prefix;
        this.priority = priority;
        this.nonBlocking = nonBlocking;
    }

    @Override
    public Thread newThread(Runnable r) {
        StringBuilder nameBuilder = new StringBuilder(prefix).append('-').append(incrementAndGet());


        String name = nameBuilder.toString();
        Thread t = nonBlocking ? new RxCustomThread(r, name) : new Thread(r, name);
        t.setPriority(priority);
        t.setDaemon(true);
        return t;
    }

    @Override
    public String toString() {
        return "RxThreadFactory[" + prefix + "]";
    }

    static final class RxCustomThread extends Thread implements NonBlockingThread {
        RxCustomThread(Runnable run, String name) {
            super(run, name);
        }
    }
}
