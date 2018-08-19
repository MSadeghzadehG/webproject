

package io.reactivex.internal.schedulers;

import java.util.concurrent.*;

import io.reactivex.disposables.Disposable;


final class DisposeOnCancel implements Future<Object> {
    final Disposable d;

    DisposeOnCancel(Disposable d) {
        this.d = d;
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        d.dispose();
        return false;
    }

    @Override
    public boolean isCancelled() {
        return false;
    }

    @Override
    public boolean isDone() {
        return false;
    }

    @Override
    public Object get() throws InterruptedException, ExecutionException {
        return null;
    }

    @Override
    public Object get(long timeout, TimeUnit unit)
            throws InterruptedException, ExecutionException, TimeoutException {
        return null;
    }
}
