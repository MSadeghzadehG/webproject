

package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Function;
import com.google.errorprone.annotations.DoNotMock;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@Beta
@DoNotMock("Use FluentFuture.from(Futures.immediate*Future) or SettableFuture")
@GwtCompatible(emulated = true)
public abstract class FluentFuture<V> extends GwtFluentFutureCatchingSpecialization<V> {
  FluentFuture() {}

  
  public static <V> FluentFuture<V> from(ListenableFuture<V> future) {
    return future instanceof FluentFuture
        ? (FluentFuture<V>) future
        : new ForwardingFluentFuture<V>(future);
  }

  
  @Partially.GwtIncompatible("AVAILABLE but requires exceptionType to be Throwable.class")
  public final <X extends Throwable> FluentFuture<V> catching(
      Class<X> exceptionType, Function<? super X, ? extends V> fallback, Executor executor) {
    return (FluentFuture<V>) Futures.catching(this, exceptionType, fallback, executor);
  }

  
  @Partially.GwtIncompatible("AVAILABLE but requires exceptionType to be Throwable.class")
  public final <X extends Throwable> FluentFuture<V> catchingAsync(
      Class<X> exceptionType, AsyncFunction<? super X, ? extends V> fallback, Executor executor) {
    return (FluentFuture<V>) Futures.catchingAsync(this, exceptionType, fallback, executor);
  }

  
  @GwtIncompatible   public final FluentFuture<V> withTimeout(
      long timeout, TimeUnit unit, ScheduledExecutorService scheduledExecutor) {
    return (FluentFuture<V>) Futures.withTimeout(this, timeout, unit, scheduledExecutor);
  }

  
  public final <T> FluentFuture<T> transformAsync(
      AsyncFunction<? super V, T> function, Executor executor) {
    return (FluentFuture<T>) Futures.transformAsync(this, function, executor);
  }

  
  public final <T> FluentFuture<T> transform(Function<? super V, T> function, Executor executor) {
    return (FluentFuture<T>) Futures.transform(this, function, executor);
  }

  
  public final void addCallback(FutureCallback<? super V> callback, Executor executor) {
    Futures.addCallback(this, callback, executor);
  }
}
