

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import java.util.concurrent.Executor;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;


@GwtCompatible
final class UncheckedThrowingFuture<V> implements ListenableFuture<V> {
  private final Error error;
  private final RuntimeException runtime;

  public static <V> ListenableFuture<V> throwingError(Error error) {
    return new UncheckedThrowingFuture<V>(error);
  }

  public static <V> ListenableFuture<V> throwingRuntimeException(RuntimeException e) {
    return new UncheckedThrowingFuture<V>(e);
  }

  private UncheckedThrowingFuture(Error error) {
    this.error = checkNotNull(error);
    this.runtime = null;
  }

  public UncheckedThrowingFuture(RuntimeException e) {
    this.runtime = checkNotNull(e);
    this.error = null;
  }

  @Override
  public boolean cancel(boolean mayInterruptIfRunning) {
    return false;
  }

  @Override
  public boolean isCancelled() {
    return false;
  }

  @Override
  public boolean isDone() {
    return true;
  }

  @Override
  public V get() {
    throwOnGet();
    throw new AssertionError("Unreachable");
  }

  @Override
  public V get(long timeout, TimeUnit unit) {
    checkNotNull(unit);
    throwOnGet();
    throw new AssertionError();
  }

  @Override
  public void addListener(Runnable listener, Executor executor) {
    checkNotNull(listener);
        executor.execute(listener);
  }

  private void throwOnGet() {
    if (error != null) {
      throw error;
    } else {
      throw runtime;
    }
  }
}
