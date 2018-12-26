

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.util.concurrent.AbstractFuture.TrustedFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(emulated = true)
abstract class ImmediateFuture<V> extends FluentFuture<V> {
  private static final Logger log = Logger.getLogger(ImmediateFuture.class.getName());

  @Override
  public void addListener(Runnable listener, Executor executor) {
    checkNotNull(listener, "Runnable was null.");
    checkNotNull(executor, "Executor was null.");
    try {
      executor.execute(listener);
    } catch (RuntimeException e) {
                  log.log(
          Level.SEVERE,
          "RuntimeException while executing runnable " + listener + " with executor " + executor,
          e);
    }
  }

  @Override
  public boolean cancel(boolean mayInterruptIfRunning) {
    return false;
  }

  @Override
  public abstract V get() throws ExecutionException;

  @Override
  public V get(long timeout, TimeUnit unit) throws ExecutionException {
    checkNotNull(unit);
    return get();
  }

  @Override
  public boolean isCancelled() {
    return false;
  }

  @Override
  public boolean isDone() {
    return true;
  }

  static class ImmediateSuccessfulFuture<V> extends ImmediateFuture<V> {
    static final ImmediateSuccessfulFuture<Object> NULL = new ImmediateSuccessfulFuture<>(null);
    @NullableDecl private final V value;

    ImmediateSuccessfulFuture(@NullableDecl V value) {
      this.value = value;
    }

        @Override
    public V get() {
      return value;
    }

    @Override
    public String toString() {
            return super.toString() + "[status=SUCCESS, result=[" + value + "]]";
    }
  }

  @GwtIncompatible   static class ImmediateSuccessfulCheckedFuture<V, X extends Exception> extends ImmediateFuture<V>
      implements CheckedFuture<V, X> {
    @NullableDecl private final V value;

    ImmediateSuccessfulCheckedFuture(@NullableDecl V value) {
      this.value = value;
    }

    @Override
    public V get() {
      return value;
    }

    @Override
    public V checkedGet() {
      return value;
    }

    @Override
    public V checkedGet(long timeout, TimeUnit unit) {
      checkNotNull(unit);
      return value;
    }

    @Override
    public String toString() {
            return super.toString() + "[status=SUCCESS, result=[" + value + "]]";
    }
  }

  static final class ImmediateFailedFuture<V> extends TrustedFuture<V> {
    ImmediateFailedFuture(Throwable thrown) {
      setException(thrown);
    }
  }

  static final class ImmediateCancelledFuture<V> extends TrustedFuture<V> {
    ImmediateCancelledFuture() {
      cancel(false);
    }
  }

  @GwtIncompatible   static class ImmediateFailedCheckedFuture<V, X extends Exception> extends ImmediateFuture<V>
      implements CheckedFuture<V, X> {
    private final X thrown;

    ImmediateFailedCheckedFuture(X thrown) {
      this.thrown = thrown;
    }

    @Override
    public V get() throws ExecutionException {
      throw new ExecutionException(thrown);
    }

    @Override
    public V checkedGet() throws X {
      throw thrown;
    }

    @Override
    public V checkedGet(long timeout, TimeUnit unit) throws X {
      checkNotNull(unit);
      throw thrown;
    }

    @Override
    public String toString() {
            return super.toString() + "[status=FAILURE, cause=[" + thrown + "]]";
    }
  }
}
