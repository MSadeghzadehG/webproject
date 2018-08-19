

package io.reactivex.internal.schedulers;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.disposables.Disposable;
import io.reactivex.internal.functions.Functions;
import io.reactivex.schedulers.SchedulerRunnableIntrospection;


abstract class AbstractDirectTask
extends AtomicReference<Future<?>>
implements Disposable, SchedulerRunnableIntrospection {

    private static final long serialVersionUID = 1811839108042568751L;

    protected final Runnable runnable;

    protected Thread runner;

    protected static final FutureTask<Void> FINISHED = new FutureTask<Void>(Functions.EMPTY_RUNNABLE, null);

    protected static final FutureTask<Void> DISPOSED = new FutureTask<Void>(Functions.EMPTY_RUNNABLE, null);

    AbstractDirectTask(Runnable runnable) {
        this.runnable = runnable;
    }

    @Override
    public final void dispose() {
        Future<?> f = get();
        if (f != FINISHED && f != DISPOSED) {
            if (compareAndSet(f, DISPOSED)) {
                if (f != null) {
                    f.cancel(runner != Thread.currentThread());
                }
            }
        }
    }

    @Override
    public final boolean isDisposed() {
        Future<?> f = get();
        return f == FINISHED || f == DISPOSED;
    }

    public final void setFuture(Future<?> future) {
        for (;;) {
            Future<?> f = get();
            if (f == FINISHED) {
                break;
            }
            if (f == DISPOSED) {
                future.cancel(runner != Thread.currentThread());
                break;
            }
            if (compareAndSet(f, future)) {
                break;
            }
        }
    }

    @Override
    public Runnable getWrappedRunnable() {
        return runnable;
    }
}
