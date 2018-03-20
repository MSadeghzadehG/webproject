

package com.google.common.testing;

import static com.google.common.base.Preconditions.checkArgument;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Ticker;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;


@Beta
@GwtCompatible
public class FakeTicker extends Ticker {

  private final AtomicLong nanos = new AtomicLong();
  private volatile long autoIncrementStepNanos;

  
  public FakeTicker advance(long time, TimeUnit timeUnit) {
    return advance(timeUnit.toNanos(time));
  }

  
  public FakeTicker advance(long nanoseconds) {
    nanos.addAndGet(nanoseconds);
    return this;
  }

  
  public FakeTicker setAutoIncrementStep(long autoIncrementStep, TimeUnit timeUnit) {
    checkArgument(autoIncrementStep >= 0, "May not auto-increment by a negative amount");
    this.autoIncrementStepNanos = timeUnit.toNanos(autoIncrementStep);
    return this;
  }

  @Override
  public long read() {
    return nanos.getAndAdd(autoIncrementStepNanos);
  }
}
