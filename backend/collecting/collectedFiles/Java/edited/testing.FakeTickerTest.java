

package com.google.common.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import java.util.EnumSet;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import junit.framework.TestCase;


@GwtCompatible(emulated = true)
public class FakeTickerTest extends TestCase {

  @GwtIncompatible   public void testNullPointerExceptions() {
    NullPointerTester tester = new NullPointerTester();
    tester.testAllPublicInstanceMethods(new FakeTicker());
  }

  public void testAdvance() {
    FakeTicker ticker = new FakeTicker();
    assertEquals(0, ticker.read());
    assertSame(ticker, ticker.advance(10));
    assertEquals(10, ticker.read());
    ticker.advance(1, TimeUnit.MILLISECONDS);
    assertEquals(1000010L, ticker.read());
  }

  public void testAutoIncrementStep_returnsSameInstance() {
    FakeTicker ticker = new FakeTicker();
    assertSame(ticker, ticker.setAutoIncrementStep(10, TimeUnit.NANOSECONDS));
  }

  public void testAutoIncrementStep_nanos() {
    FakeTicker ticker = new FakeTicker().setAutoIncrementStep(10, TimeUnit.NANOSECONDS);
    assertEquals(0, ticker.read());
    assertEquals(10, ticker.read());
    assertEquals(20, ticker.read());
  }

  public void testAutoIncrementStep_millis() {
    FakeTicker ticker = new FakeTicker().setAutoIncrementStep(1, TimeUnit.MILLISECONDS);
    assertEquals(0, ticker.read());
    assertEquals(1000000, ticker.read());
    assertEquals(2000000, ticker.read());
  }

  public void testAutoIncrementStep_seconds() {
    FakeTicker ticker = new FakeTicker().setAutoIncrementStep(3, TimeUnit.SECONDS);
    assertEquals(0, ticker.read());
    assertEquals(3000000000L, ticker.read());
    assertEquals(6000000000L, ticker.read());
  }

  public void testAutoIncrementStep_resetToZero() {
    FakeTicker ticker = new FakeTicker().setAutoIncrementStep(10, TimeUnit.NANOSECONDS);
    assertEquals(0, ticker.read());
    assertEquals(10, ticker.read());
    assertEquals(20, ticker.read());

    for (TimeUnit timeUnit : EnumSet.allOf(TimeUnit.class)) {
      ticker.setAutoIncrementStep(0, timeUnit);
      assertEquals(
          "Expected no auto-increment when setting autoIncrementStep to 0 " + timeUnit,
          30,
          ticker.read());
    }
  }

  public void testAutoIncrement_negative() {
    FakeTicker ticker = new FakeTicker();
    try {
      ticker.setAutoIncrementStep(-1, TimeUnit.NANOSECONDS);
      fail("Expected IllegalArgumentException");
    } catch (IllegalArgumentException expected) {
    }
  }

  @GwtIncompatible 
  public void testConcurrentAdvance() throws Exception {
    final FakeTicker ticker = new FakeTicker();

    int numberOfThreads = 64;
    runConcurrentTest(
        numberOfThreads,
        new Callable<Void>() {
          @Override
          public Void call() throws Exception {
                        ticker.advance(1L);
            Thread.sleep(10);
            ticker.advance(1L);
            return null;
          }
        });

    assertEquals(numberOfThreads * 2, ticker.read());
  }

  @GwtIncompatible 
  public void testConcurrentAutoIncrementStep() throws Exception {
    int incrementByNanos = 3;
    final FakeTicker ticker =
        new FakeTicker().setAutoIncrementStep(incrementByNanos, TimeUnit.NANOSECONDS);

    int numberOfThreads = 64;
    runConcurrentTest(
        numberOfThreads,
        new Callable<Void>() {
          @Override
          public Void call() throws Exception {
            ticker.read();
            return null;
          }
        });

    assertEquals(incrementByNanos * numberOfThreads, ticker.read());
  }

  
  @GwtIncompatible   private void runConcurrentTest(int numberOfThreads, final Callable<Void> callable)
      throws Exception {
    ExecutorService executorService = Executors.newFixedThreadPool(numberOfThreads);
    final CountDownLatch startLatch = new CountDownLatch(numberOfThreads);
    final CountDownLatch doneLatch = new CountDownLatch(numberOfThreads);
    for (int i = numberOfThreads; i > 0; i--) {
      @SuppressWarnings("unused")       Future<?> possiblyIgnoredError =
          executorService.submit(
              new Callable<Void>() {
                @Override
                public Void call() throws Exception {
                  startLatch.countDown();
                  startLatch.await();
                  callable.call();
                  doneLatch.countDown();
                  return null;
                }
              });
    }
    doneLatch.await();
  }
}
