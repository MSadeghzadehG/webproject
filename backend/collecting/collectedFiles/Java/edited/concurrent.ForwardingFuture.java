

package com.google.common.util.concurrent;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Preconditions;
import com.google.common.collect.ForwardingObject;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@CanIgnoreReturnValue @GwtCompatible
public abstract class ForwardingFuture<V> extends ForwardingObject implements Future<V> {
  
  protected ForwardingFuture() {}

  @Override
  protected abstract Future<? extends V> delegate();

  @Override
  public boolean cancel(boolean mayInterruptIfRunning) {
    return delegate().cancel(mayInterruptIfRunning);
  }

  @Override
  public boolean isCancelled() {
    return delegate().isCancelled();
  }

  @Override
  public boolean isDone() {
    return delegate().isDone();
  }

  @Override
  public V get() throws InterruptedException, ExecutionException {
    return delegate().get();
  }

  @Override
  public V get(long timeout, TimeUnit unit)
      throws InterruptedException, ExecutionException, TimeoutException {
    return delegate().get(timeout, unit);
  }

    
  public abstract static class SimpleForwardingFuture<V> extends ForwardingFuture<V> {
    private final Future<V> delegate;

    protected SimpleForwardingFuture(Future<V> delegate) {
      this.delegate = Preconditions.checkNotNull(delegate);
    }

    @Override
    protected final Future<V> delegate() {
      return delegate;
    }
  }
}
