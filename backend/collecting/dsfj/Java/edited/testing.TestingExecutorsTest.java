

package com.google.common.util.concurrent.testing;

import com.google.common.collect.ImmutableList;
import com.google.common.util.concurrent.ListeningScheduledExecutorService;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import junit.framework.TestCase;


public class TestingExecutorsTest extends TestCase {
  private volatile boolean taskDone;

  public void testNoOpScheduledExecutor() throws InterruptedException {
    taskDone = false;
    Runnable task =
        new Runnable() {
          @Override
          public void run() {
            taskDone = true;
          }
        };
    ScheduledFuture<?> future =
        TestingExecutors.noOpScheduledExecutor().schedule(task, 10, TimeUnit.MILLISECONDS);
    Thread.sleep(20);
    assertFalse(taskDone);
    assertFalse(future.isDone());
  }

  public void testNoOpScheduledExecutorShutdown() {
    ListeningScheduledExecutorService executor = TestingExecutors.noOpScheduledExecutor();
    assertFalse(executor.isShutdown());
    assertFalse(executor.isTerminated());
    executor.shutdown();
    assertTrue(executor.isShutdown());
    assertTrue(executor.isTerminated());
  }

  public void testNoOpScheduledExecutorInvokeAll() throws ExecutionException, InterruptedException {
    ListeningScheduledExecutorService executor = TestingExecutors.noOpScheduledExecutor();
    taskDone = false;
    Callable<Boolean> task =
        new Callable<Boolean>() {
          @Override
          public Boolean call() {
            taskDone = true;
            return taskDone;
          }
        };
    List<Future<Boolean>> futureList =
        executor.invokeAll(ImmutableList.of(task), 10, TimeUnit.MILLISECONDS);
    Future<Boolean> future = futureList.get(0);
    assertFalse(taskDone);
    assertTrue(future.isDone());
    try {
      future.get();
      fail();
    } catch (CancellationException e) {
          }
  }

  public void testSameThreadScheduledExecutor() throws ExecutionException, InterruptedException {
    taskDone = false;
    Callable<Integer> task =
        new Callable<Integer>() {
          @Override
          public Integer call() {
            taskDone = true;
            return 6;
          }
        };
    Future<Integer> future =
        TestingExecutors.sameThreadScheduledExecutor().schedule(task, 10000, TimeUnit.MILLISECONDS);
    assertTrue("Should run callable immediately", taskDone);
    assertEquals(6, (int) future.get());
  }

  public void testSameThreadScheduledExecutorWithException() throws InterruptedException {
    Runnable runnable =
        new Runnable() {
          @Override
          public void run() {
            throw new RuntimeException("Oh no!");
          }
        };

    Future<?> future = TestingExecutors.sameThreadScheduledExecutor().submit(runnable);
    try {
      future.get();
      fail("Should have thrown exception");
    } catch (ExecutionException e) {
          }
  }
}
