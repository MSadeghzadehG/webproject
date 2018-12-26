

package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import java.util.concurrent.Callable;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;


@Beta
@GwtIncompatible
public interface ListeningScheduledExecutorService
    extends ScheduledExecutorService, ListeningExecutorService {

  
  @Override
  ListenableScheduledFuture<?> schedule(Runnable command, long delay, TimeUnit unit);

  
  @Override
  <V> ListenableScheduledFuture<V> schedule(Callable<V> callable, long delay, TimeUnit unit);

  
  @Override
  ListenableScheduledFuture<?> scheduleAtFixedRate(
      Runnable command, long initialDelay, long period, TimeUnit unit);

  
  @Override
  ListenableScheduledFuture<?> scheduleWithFixedDelay(
      Runnable command, long initialDelay, long delay, TimeUnit unit);
}
