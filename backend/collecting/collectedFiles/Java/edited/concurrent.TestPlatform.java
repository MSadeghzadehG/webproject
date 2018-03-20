

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkState;
import static com.google.common.truth.Truth.assertThat;
import static java.util.concurrent.TimeUnit.SECONDS;
import static junit.framework.Assert.fail;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeoutException;


final class TestPlatform {
  static void verifyGetOnPendingFuture(Future<?> future) {
    try {
      future.get();
      fail();
    } catch (Exception e) {
      assertThat(e).isInstanceOf(IllegalStateException.class);
      assertThat(e).hasMessage("Cannot get() on a pending future.");
    }
  }

  static void verifyTimedGetOnPendingFuture(Future<?> future) {
    try {
      future.get(0, SECONDS);
      fail();
    } catch (Exception e) {
      assertThat(e).isInstanceOf(IllegalStateException.class);
      assertThat(e).hasMessage("Cannot get() on a pending future.");
    }
  }

  static void verifyThreadWasNotInterrupted() {
      }

  static void clearInterrupt() {
      }

  static <V> V getDoneFromTimeoutOverload(Future<V> future) throws ExecutionException {
    checkState(future.isDone(), "Future was expected to be done: %s", future);
    try {
      return future.get(0, SECONDS);
    } catch (InterruptedException e) {
      throw new AssertionError();
    } catch (TimeoutException e) {
      throw new AssertionError();
    }
  }

  private TestPlatform() {}
}
