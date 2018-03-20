

package io.reactivex.internal.schedulers;

import io.reactivex.plugins.RxJavaPlugins;


public final class ScheduledDirectPeriodicTask extends AbstractDirectTask implements Runnable {

    private static final long serialVersionUID = 1811839108042568751L;

    public ScheduledDirectPeriodicTask(Runnable runnable) {
        super(runnable);
    }

    @Override
    public void run() {
        runner = Thread.currentThread();
        try {
            runnable.run();
            runner = null;
        } catch (Throwable ex) {
            runner = null;
            lazySet(FINISHED);
            RxJavaPlugins.onError(ex);
        }
    }
}
