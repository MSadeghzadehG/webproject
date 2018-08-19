

package io.reactivex.internal.schedulers;

import java.util.concurrent.Callable;


public final class ScheduledDirectTask extends AbstractDirectTask implements Callable<Void> {

    private static final long serialVersionUID = 1811839108042568751L;

    public ScheduledDirectTask(Runnable runnable) {
        super(runnable);
    }

    @Override
    public Void call() throws Exception {
        runner = Thread.currentThread();
        try {
            runnable.run();
        } finally {
            lazySet(FINISHED);
            runner = null;
        }
        return null;
    }
}
