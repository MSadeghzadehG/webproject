

package io.reactivex.internal.schedulers;

import java.util.concurrent.TimeUnit;

import io.reactivex.Scheduler;
import io.reactivex.annotations.NonNull;
import io.reactivex.disposables.*;


public final class ImmediateThinScheduler extends Scheduler {

    
    public static final Scheduler INSTANCE = new ImmediateThinScheduler();

    static final Worker WORKER = new ImmediateThinWorker();

    static final Disposable DISPOSED;

    static {
        DISPOSED = Disposables.empty();
        DISPOSED.dispose();
    }

    private ImmediateThinScheduler() {
            }

    @NonNull
    @Override
    public Disposable scheduleDirect(@NonNull Runnable run) {
        run.run();
        return DISPOSED;
    }

    @NonNull
    @Override
    public Disposable scheduleDirect(@NonNull Runnable run, long delay, TimeUnit unit) {
        throw new UnsupportedOperationException("This scheduler doesn't support delayed execution");
    }

    @NonNull
    @Override
    public Disposable schedulePeriodicallyDirect(@NonNull Runnable run, long initialDelay, long period, TimeUnit unit) {
        throw new UnsupportedOperationException("This scheduler doesn't support periodic execution");
    }

    @NonNull
    @Override
    public Worker createWorker() {
        return WORKER;
    }

    static final class ImmediateThinWorker extends Worker {

        @Override
        public void dispose() {
                    }

        @Override
        public boolean isDisposed() {
            return false;         }

        @NonNull
        @Override
        public Disposable schedule(@NonNull Runnable run) {
            run.run();
            return DISPOSED;
        }

        @NonNull
        @Override
        public Disposable schedule(@NonNull Runnable run, long delay, @NonNull TimeUnit unit) {
            throw new UnsupportedOperationException("This scheduler doesn't support delayed execution");
        }

        @NonNull
        @Override
        public Disposable schedulePeriodically(@NonNull Runnable run, long initialDelay, long period, TimeUnit unit) {
            throw new UnsupportedOperationException("This scheduler doesn't support periodic execution");
        }
    }
}
