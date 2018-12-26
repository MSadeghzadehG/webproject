

package com.google.common.util.concurrent.testing;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.util.concurrent.ListenableFuture;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import junit.framework.TestCase;


@Beta
@GwtIncompatible
public abstract class AbstractListenableFutureTest extends TestCase {

  protected CountDownLatch latch;
  protected ListenableFuture<Boolean> future;

  @Override
  protected void setUp() throws Exception {

        latch = new CountDownLatch(1);
    future = createListenableFuture(Boolean.TRUE, null, latch);
  }

  @Override
  protected void tearDown() throws Exception {

        latch.countDown();
  }

  
  protected abstract <V> ListenableFuture<V> createListenableFuture(
      V value, Exception except, CountDownLatch waitOn);

  
  public void testGetBlocksUntilValueAvailable() throws Throwable {

    assertFalse(future.isDone());
    assertFalse(future.isCancelled());

    final CountDownLatch successLatch = new CountDownLatch(1);
    final Throwable[] badness = new Throwable[1];

        new Thread(
            new Runnable() {
              @Override
              public void run() {
                try {
                  assertSame(Boolean.TRUE, future.get());
                  successLatch.countDown();
                } catch (Throwable t) {
                  t.printStackTrace();
                  badness[0] = t;
                }
              }
            })
        .start();

        latch.countDown();

    assertTrue(successLatch.await(10, TimeUnit.SECONDS));

    if (badness[0] != null) {
      throw badness[0];
    }

    assertTrue(future.isDone());
    assertFalse(future.isCancelled());
  }

  
  public void testTimeoutOnGetWorksCorrectly() throws InterruptedException, ExecutionException {

        try {
      future.get(20, TimeUnit.MILLISECONDS);
      fail("Should have timed out trying to get the value.");
    } catch (TimeoutException expected) {
    } finally {
      latch.countDown();
    }
  }

  
  public void testCanceledFutureThrowsCancellation() throws Exception {

    assertFalse(future.isDone());
    assertFalse(future.isCancelled());

    final CountDownLatch successLatch = new CountDownLatch(1);

        new Thread(
            new Runnable() {
              @Override
              public void run() {
                try {
                  future.get();
                } catch (CancellationException expected) {
                  successLatch.countDown();
                } catch (Exception ignored) {
                                  }
              }
            })
        .start();

    assertFalse(future.isDone());
    assertFalse(future.isCancelled());

    future.cancel(true);

    assertTrue(future.isDone());
    assertTrue(future.isCancelled());

    assertTrue(successLatch.await(200, TimeUnit.MILLISECONDS));

    latch.countDown();
  }

  public void testListenersNotifiedOnError() throws Exception {
    final CountDownLatch successLatch = new CountDownLatch(1);
    final CountDownLatch listenerLatch = new CountDownLatch(1);

    ExecutorService exec = Executors.newCachedThreadPool();

    future.addListener(
        new Runnable() {
          @Override
          public void run() {
            listenerLatch.countDown();
          }
        },
        exec);

    new Thread(
            new Runnable() {
              @Override
              public void run() {
                try {
                  future.get();
                } catch (CancellationException expected) {
                  successLatch.countDown();
                } catch (Exception ignored) {
                                  }
              }
            })
        .start();

    future.cancel(true);

    assertTrue(future.isCancelled());
    assertTrue(future.isDone());

    assertTrue(successLatch.await(200, TimeUnit.MILLISECONDS));
    assertTrue(listenerLatch.await(200, TimeUnit.MILLISECONDS));

    latch.countDown();

    exec.shutdown();
    exec.awaitTermination(100, TimeUnit.MILLISECONDS);
  }

  
  public void testAllListenersCompleteSuccessfully()
      throws InterruptedException, ExecutionException {

    ExecutorService exec = Executors.newCachedThreadPool();

    int listenerCount = 20;
    final CountDownLatch listenerLatch = new CountDownLatch(listenerCount);

            for (int i = 0; i < 20; i++) {

            if (i == 10) {
        new Thread(
                new Runnable() {
                  @Override
                  public void run() {
                    latch.countDown();
                  }
                })
            .start();
      }

      future.addListener(
          new Runnable() {
            @Override
            public void run() {
              listenerLatch.countDown();
            }
          },
          exec);
    }

    assertSame(Boolean.TRUE, future.get());
        listenerLatch.await(500, TimeUnit.MILLISECONDS);

    exec.shutdown();
    exec.awaitTermination(500, TimeUnit.MILLISECONDS);
  }
}
