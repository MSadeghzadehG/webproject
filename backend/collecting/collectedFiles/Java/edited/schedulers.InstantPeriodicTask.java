

package io.reactivex.internal.schedulers;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.disposables.Disposable;
import io.reactivex.internal.functions.Functions;
import io.reactivex.plugins.RxJavaPlugins;


final class InstantPeriodicTask implements Callable<Void>, Disposable {

    final Runnable task;

    final AtomicReference<Future<?>> rest;

    final AtomicReference<Future<?>> first;

    final ExecutorService executor;

    Thread runner;

    static final FutureTask<Void> CANCELLED = new FutureTask<Void>(Functions.EMPTY_RUNNABLE, null);

    InstantPeriodicTask(Runnable task, ExecutorService executor) {
        super();
        this.task = task;
        this.first = new AtomicReference<Future<?>>();
        this.rest = new AtomicReference<Future<?>>();
        this.executor = executor;
    }

    @Override
    public Void call() throws Exception {
        runner = Thread.currentThread();
        try {
            task.run();
            setRest(executor.submit(this));
            runner = null;
        } catch (Throwable ex) {
            runner = null;
            RxJavaPlugins.onError(ex);
        }
        return null;
    }

    @Override
    public void dispose() {
        Future<?> current = first.getAndSet(CANCELLED);
        if (current != null && current != CANCELLED) {
            current.cancel(runner != Thread.currentThread());
        }
        current = rest.getAndSet(CANCELLED);
        if (current != null && current != CANCELLED) {
            current.cancel(runner != Thread.currentThread());
        }
    }

    @Override
    public boolean isDisposed() {
        return first.get() == CANCELLED;
    }

    void setFirst(Future<?> f) {
        for (;;) {
            Future<?> current = first.get();
            if (current == CANCELLED) {
                f.cancel(runner != Thread.currentThread());
                return;
            }
            if (first.compareAndSet(current, f)) {
                return;
            }
        }
    }

    void setRest(Future<?> f) {
        for (;;) {
            Future<?> current = rest.get();
            if (current == CANCELLED) {
                f.cancel(runner != Thread.currentThread());
                return;
            }
            if (rest.compareAndSet(current, f)) {
                return;
            }
        }
    }
}
