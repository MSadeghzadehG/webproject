

package com.google.common.util.concurrent;

import java.util.Date;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;


abstract class ForwardingCondition implements Condition {
  abstract Condition delegate();

  @Override
  public void await() throws InterruptedException {
    delegate().await();
  }

  @Override
  public boolean await(long time, TimeUnit unit) throws InterruptedException {
    return delegate().await(time, unit);
  }

  @Override
  public void awaitUninterruptibly() {
    delegate().awaitUninterruptibly();
  }

  @Override
  public long awaitNanos(long nanosTimeout) throws InterruptedException {
    return delegate().awaitNanos(nanosTimeout);
  }

  @Override
  public boolean awaitUntil(Date deadline) throws InterruptedException {
    return delegate().awaitUntil(deadline);
  }

  @Override
  public void signal() {
    delegate().signal();
  }

  @Override
  public void signalAll() {
    delegate().signalAll();
  }
}
