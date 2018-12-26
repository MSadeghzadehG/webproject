

package com.google.common.util.concurrent.testing;

import static com.google.common.util.concurrent.MoreExecutors.directExecutor;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.util.concurrent.ListenableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import junit.framework.Assert;


@Beta
@GwtIncompatible
public class MockFutureListener implements Runnable {
  private final CountDownLatch countDownLatch;
  private final ListenableFuture<?> future;

  public MockFutureListener(ListenableFuture<?> future) {
    this.countDownLatch = new CountDownLatch(1);
    this.future = future;

    future.addListener(this, directExecutor());
  }

  @Override
  public void run() {
    countDownLatch.countDown();
  }

  
  public void assertSuccess(Object expectedData) throws Throwable {
        Assert.assertTrue(countDownLatch.await(1L, TimeUnit.SECONDS));

    try {
      Assert.assertEquals(expectedData, future.get());
    } catch (ExecutionException e) {
      throw e.getCause();
    }
  }

  
  public void assertException(Throwable expectedCause) throws Exception {
        Assert.assertTrue(countDownLatch.await(1L, TimeUnit.SECONDS));

    try {
      future.get();
      Assert.fail("This call was supposed to throw an ExecutionException");
    } catch (ExecutionException expected) {
      Assert.assertSame(expectedCause, expected.getCause());
    }
  }

  public void assertTimeout() throws Exception {
            Assert.assertFalse(countDownLatch.await(1L, TimeUnit.SECONDS));
  }
}
