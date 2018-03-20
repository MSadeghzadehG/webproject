

package com.google.common.util.concurrent;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.ImmutableList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import junit.framework.TestCase;


public class AbstractListeningExecutorServiceTest extends TestCase {

  public void testSubmit() throws Exception {
    

    TestListeningExecutorService e = new TestListeningExecutorService();

    TestRunnable runnable = new TestRunnable();
    ListenableFuture<?> runnableFuture = e.submit(runnable);
    assertThat(runnableFuture).isInstanceOf(TrustedListenableFutureTask.class);
    assertTrue(runnableFuture.isDone());
    assertTrue(runnable.run);

    ListenableFuture<String> callableFuture = e.submit(new TestCallable());
    assertThat(callableFuture).isInstanceOf(TrustedListenableFutureTask.class);
    assertTrue(callableFuture.isDone());
    assertEquals("foo", callableFuture.get());

    TestRunnable runnable2 = new TestRunnable();
    ListenableFuture<Integer> runnableFuture2 = e.submit(runnable2, 3);
    assertThat(runnableFuture2).isInstanceOf(TrustedListenableFutureTask.class);
    assertTrue(runnableFuture2.isDone());
    assertTrue(runnable2.run);
    assertEquals((Integer) 3, runnableFuture2.get());
  }

  private static class TestRunnable implements Runnable {
    boolean run = false;

    @Override
    public void run() {
      run = true;
    }
  }

  private static class TestCallable implements Callable<String> {
    @Override
    public String call() {
      return "foo";
    }
  }

  
  private static class TestListeningExecutorService extends AbstractListeningExecutorService {

    @Override
    public void execute(Runnable runnable) {
      assertThat(runnable).isInstanceOf(TrustedListenableFutureTask.class);
      runnable.run();
    }

    @Override
    public void shutdown() {}

    @Override
    public List<Runnable> shutdownNow() {
      return ImmutableList.of();
    }

    @Override
    public boolean isShutdown() {
      return false;
    }

    @Override
    public boolean isTerminated() {
      return false;
    }

    @Override
    public boolean awaitTermination(long timeout, TimeUnit unit) {
      return false;
    }
  }
}
