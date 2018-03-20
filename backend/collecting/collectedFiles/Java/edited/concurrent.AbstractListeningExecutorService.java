

package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.AbstractExecutorService;
import java.util.concurrent.Callable;
import java.util.concurrent.RunnableFuture;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@CanIgnoreReturnValue
@GwtIncompatible
public abstract class AbstractListeningExecutorService extends AbstractExecutorService
    implements ListeningExecutorService {

  
  @Override
  protected final <T> RunnableFuture<T> newTaskFor(Runnable runnable, T value) {
    return TrustedListenableFutureTask.create(runnable, value);
  }

  
  @Override
  protected final <T> RunnableFuture<T> newTaskFor(Callable<T> callable) {
    return TrustedListenableFutureTask.create(callable);
  }

  @Override
  public ListenableFuture<?> submit(Runnable task) {
    return (ListenableFuture<?>) super.submit(task);
  }

  @Override
  public <T> ListenableFuture<T> submit(Runnable task, @NullableDecl T result) {
    return (ListenableFuture<T>) super.submit(task, result);
  }

  @Override
  public <T> ListenableFuture<T> submit(Callable<T> task) {
    return (ListenableFuture<T>) super.submit(task);
  }
}
