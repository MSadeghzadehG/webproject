

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Throwables.throwIfUnchecked;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.ImmutableList;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@CanIgnoreReturnValue @GwtIncompatible
abstract class WrappingExecutorService implements ExecutorService {
  private final ExecutorService delegate;

  protected WrappingExecutorService(ExecutorService delegate) {
    this.delegate = checkNotNull(delegate);
  }

  
  protected abstract <T> Callable<T> wrapTask(Callable<T> callable);

  
  protected Runnable wrapTask(Runnable command) {
    final Callable<Object> wrapped = wrapTask(Executors.callable(command, null));
    return new Runnable() {
      @Override
      public void run() {
        try {
          wrapped.call();
        } catch (Exception e) {
          throwIfUnchecked(e);
          throw new RuntimeException(e);
        }
      }
    };
  }

  
  private <T> ImmutableList<Callable<T>> wrapTasks(Collection<? extends Callable<T>> tasks) {
    ImmutableList.Builder<Callable<T>> builder = ImmutableList.builder();
    for (Callable<T> task : tasks) {
      builder.add(wrapTask(task));
    }
    return builder.build();
  }

    @Override
  public final void execute(Runnable command) {
    delegate.execute(wrapTask(command));
  }

  @Override
  public final <T> Future<T> submit(Callable<T> task) {
    return delegate.submit(wrapTask(checkNotNull(task)));
  }

  @Override
  public final Future<?> submit(Runnable task) {
    return delegate.submit(wrapTask(task));
  }

  @Override
  public final <T> Future<T> submit(Runnable task, T result) {
    return delegate.submit(wrapTask(task), result);
  }

  @Override
  public final <T> List<Future<T>> invokeAll(Collection<? extends Callable<T>> tasks)
      throws InterruptedException {
    return delegate.invokeAll(wrapTasks(tasks));
  }

  @Override
  public final <T> List<Future<T>> invokeAll(
      Collection<? extends Callable<T>> tasks, long timeout, TimeUnit unit)
      throws InterruptedException {
    return delegate.invokeAll(wrapTasks(tasks), timeout, unit);
  }

  @Override
  public final <T> T invokeAny(Collection<? extends Callable<T>> tasks)
      throws InterruptedException, ExecutionException {
    return delegate.invokeAny(wrapTasks(tasks));
  }

  @Override
  public final <T> T invokeAny(Collection<? extends Callable<T>> tasks, long timeout, TimeUnit unit)
      throws InterruptedException, ExecutionException, TimeoutException {
    return delegate.invokeAny(wrapTasks(tasks), timeout, unit);
  }

  
  @Override
  public final void shutdown() {
    delegate.shutdown();
  }

  @Override
  public final List<Runnable> shutdownNow() {
    return delegate.shutdownNow();
  }

  @Override
  public final boolean isShutdown() {
    return delegate.isShutdown();
  }

  @Override
  public final boolean isTerminated() {
    return delegate.isTerminated();
  }

  @Override
  public final boolean awaitTermination(long timeout, TimeUnit unit) throws InterruptedException {
    return delegate.awaitTermination(timeout, unit);
  }
}
