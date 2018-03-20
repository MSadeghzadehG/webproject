

package com.google.common.util.concurrent;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.util.concurrent.Futures.getDone;
import static com.google.common.util.concurrent.Futures.immediateCancelledFuture;
import static com.google.common.util.concurrent.Futures.immediateFailedFuture;
import static com.google.common.util.concurrent.Futures.immediateFuture;

import com.google.common.annotations.GwtCompatible;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import junit.framework.TestCase;


@GwtCompatible
public class FuturesGetDoneTest extends TestCase {
  public void testSuccessful() throws ExecutionException {
    assertThat(getDone(immediateFuture("a"))).isEqualTo("a");
  }

  public void testSuccessfulNull() throws ExecutionException {
    assertThat(getDone(immediateFuture((String) null))).isEqualTo(null);
  }

  public void testFailed() {
    Exception failureCause = new Exception();
    try {
      getDone(immediateFailedFuture(failureCause));
      fail();
    } catch (ExecutionException expected) {
      assertThat(expected.getCause()).isEqualTo(failureCause);
    }
  }

  public void testCancelled() throws ExecutionException {
    try {
      getDone(immediateCancelledFuture());
      fail();
    } catch (CancellationException expected) {
    }
  }

  public void testPending() throws ExecutionException {
    try {
      getDone(SettableFuture.create());
      fail();
    } catch (IllegalStateException expected) {
    }
  }
}
