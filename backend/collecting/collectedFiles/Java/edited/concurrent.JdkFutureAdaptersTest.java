

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkState;
import static com.google.common.util.concurrent.Futures.immediateFuture;
import static com.google.common.util.concurrent.JdkFutureAdapters.listenInPoolThread;
import static com.google.common.util.concurrent.MoreExecutors.directExecutor;
import static java.util.concurrent.Executors.newCachedThreadPool;
import static java.util.concurrent.TimeUnit.SECONDS;

import com.google.common.testing.ClassSanityTester;
import com.google.common.util.concurrent.FuturesTest.ExecutorSpy;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import junit.framework.AssertionFailedError;
import junit.framework.TestCase;


public class JdkFutureAdaptersTest extends TestCase {
  private static final String DATA1 = "data";

  public void testListenInPoolThreadReturnsSameFuture() throws Exception {
    ListenableFuture<String> listenableFuture = immediateFuture(DATA1);
    assertSame(listenableFuture, listenInPoolThread(listenableFuture));
  }

  private static class SingleCallListener implements Runnable {

    private boolean expectCall = false;
    private final CountDownLatch calledCountDown = new CountDownLatch(1);

    @Override
    public void run() {
      assertTrue("Listener called before it was expected", expectCall);
      assertFalse("Listener called more than once", wasCalled());
      calledCountDown.countDown();
    }

    public void expectCall() {
      assertFalse("expectCall is already true", expectCall);
      expectCall = true;
    }

    public boolean wasCalled() {
      return calledCountDown.getCount() == 0;
    }

    public void waitForCall() throws InterruptedException {
      assertTrue("expectCall is false", expectCall);
      calledCountDown.await();
    }
  }

  public void testListenInPoolThreadIgnoresExecutorWhenDelegateIsDone() throws Exception {
    NonListenableSettableFuture<String> abstractFuture = NonListenableSettableFuture.create();
    abstractFuture.set(DATA1);
    ExecutorSpy spy = new ExecutorSpy(directExecutor());
    ListenableFuture<String> listenableFuture = listenInPoolThread(abstractFuture, spy);

    SingleCallListener singleCallListener = new SingleCallListener();
    singleCallListener.expectCall();

    assertFalse(spy.wasExecuted);
    assertFalse(singleCallListener.wasCalled());
    assertTrue(listenableFuture.isDone()); 
            listenableFuture.addListener(singleCallListener, directExecutor());
    assertEquals(DATA1, listenableFuture.get());

            assertFalse(spy.wasExecuted);
    assertTrue(singleCallListener.wasCalled());
    assertTrue(listenableFuture.isDone());
  }

  public void testListenInPoolThreadUsesGivenExecutor() throws Exception {
    ExecutorService executorService =
        newCachedThreadPool(new ThreadFactoryBuilder().setDaemon(true).build());
    NonListenableSettableFuture<String> abstractFuture = NonListenableSettableFuture.create();
    ExecutorSpy spy = new ExecutorSpy(executorService);
    ListenableFuture<String> listenableFuture = listenInPoolThread(abstractFuture, spy);

    SingleCallListener singleCallListener = new SingleCallListener();
    singleCallListener.expectCall();

    assertFalse(spy.wasExecuted);
    assertFalse(singleCallListener.wasCalled());
    assertFalse(listenableFuture.isDone());

    listenableFuture.addListener(singleCallListener, executorService);
    abstractFuture.set(DATA1);
    assertEquals(DATA1, listenableFuture.get());
    singleCallListener.waitForCall();

    assertTrue(spy.wasExecuted);
    assertTrue(singleCallListener.wasCalled());
    assertTrue(listenableFuture.isDone());
  }

  public void testListenInPoolThreadCustomExecutorInterrupted() throws Exception {
    final CountDownLatch submitSuccessful = new CountDownLatch(1);
    ExecutorService executorService =
        new ThreadPoolExecutor(
            0,
            Integer.MAX_VALUE,
            60L,
            TimeUnit.SECONDS,
            new SynchronousQueue<Runnable>(),
            new ThreadFactoryBuilder().setDaemon(true).build()) {
          @Override
          protected void beforeExecute(Thread t, Runnable r) {
            submitSuccessful.countDown();
          }
        };
    NonListenableSettableFuture<String> abstractFuture = NonListenableSettableFuture.create();
    ListenableFuture<String> listenableFuture = listenInPoolThread(abstractFuture, executorService);

    SingleCallListener singleCallListener = new SingleCallListener();
    singleCallListener.expectCall();

    assertFalse(singleCallListener.wasCalled());
    assertFalse(listenableFuture.isDone());

    listenableFuture.addListener(singleCallListener, directExecutor());
    
    submitSuccessful.await();
    executorService.shutdownNow();
    abstractFuture.set(DATA1);
    assertEquals(DATA1, listenableFuture.get());
    singleCallListener.waitForCall();

    assertTrue(singleCallListener.wasCalled());
    assertTrue(listenableFuture.isDone());
  }

  
  private static final class NonListenableSettableFuture<V> extends ForwardingFuture<V> {
    static <V> NonListenableSettableFuture<V> create() {
      return new NonListenableSettableFuture<V>();
    }

    final SettableFuture<V> delegate = SettableFuture.create();

    @Override
    protected Future<V> delegate() {
      return delegate;
    }

    void set(V value) {
      delegate.set(value);
    }
  }

  private static final class RuntimeExceptionThrowingFuture<V> implements Future<V> {
    final CountDownLatch allowGetToComplete = new CountDownLatch(1);

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
      throw new AssertionFailedError();
    }

    @Override
    public V get() throws InterruptedException {
      
      allowGetToComplete.await(1, SECONDS);
      throw new RuntimeException("expected, should be caught");
    }

    @Override
    public V get(long timeout, TimeUnit unit) {
      throw new AssertionFailedError();
    }

    @Override
    public boolean isCancelled() {
      throw new AssertionFailedError();
    }

    @Override
    public boolean isDone() {
      
      return false;
    }
  }

  private static final class RecordingRunnable implements Runnable {
    final CountDownLatch wasRun = new CountDownLatch(1);

        @Override
    public synchronized void run() {
      checkState(wasRun.getCount() > 0);
      wasRun.countDown();
    }
  }

  public void testListenInPoolThreadRunsListenerAfterRuntimeException() throws Exception {
    RuntimeExceptionThrowingFuture<String> input = new RuntimeExceptionThrowingFuture<>();
    
    assertFalse(
        "Can't test the main listenInPoolThread path "
            + "if the input is already a ListenableFuture",
        ListenableFuture.class.isInstance(input));
    ListenableFuture<String> listenable = listenInPoolThread(input);
    
    RecordingRunnable earlyListener = new RecordingRunnable();
    listenable.addListener(earlyListener, directExecutor());

    input.allowGetToComplete.countDown();
        assertTrue(earlyListener.wasRun.await(1, SECONDS));

        RecordingRunnable lateListener = new RecordingRunnable();
    listenable.addListener(lateListener, directExecutor());
    assertTrue(lateListener.wasRun.await(1, SECONDS));
  }

  public void testAdapters_nullChecks() throws Exception {
    new ClassSanityTester()
        .forAllPublicStaticMethods(JdkFutureAdapters.class)
        .thatReturn(Future.class)
        .testNulls();
  }
}
