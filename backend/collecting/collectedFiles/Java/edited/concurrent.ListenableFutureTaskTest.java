

package com.google.common.util.concurrent;

import static com.google.common.util.concurrent.MoreExecutors.directExecutor;

import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import junit.framework.TestCase;


public class ListenableFutureTaskTest extends TestCase {

  private ExecutorService exec;

  protected final CountDownLatch runLatch = new CountDownLatch(1);
  protected final CountDownLatch taskLatch = new CountDownLatch(1);
  protected final CountDownLatch listenerLatch = new CountDownLatch(1);

  protected volatile boolean throwException = false;

  protected final ListenableFutureTask<Integer> task =
      ListenableFutureTask.create(
          new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
              runLatch.countDown();
              taskLatch.await();
              if (throwException) {
                throw new IllegalStateException("Fail");
              }
              return 25;
            }
          });

  @Override
  protected void setUp() throws Exception {
    super.setUp();

    exec = Executors.newCachedThreadPool();

    task.addListener(
        new Runnable() {
          @Override
          public void run() {
            listenerLatch.countDown();
          }
        },
        directExecutor());
  }

  @Override
  protected void tearDown() throws Exception {
    if (exec != null) {
      exec.shutdown();
    }

    super.tearDown();
  }

  public void testListenerDoesNotRunUntilTaskCompletes() throws Exception {

        assertEquals(1, listenerLatch.getCount());
    assertFalse(task.isDone());
    assertFalse(task.isCancelled());

                exec.execute(task);
    runLatch.await();
    assertEquals(1, listenerLatch.getCount());
    assertFalse(task.isDone());
    assertFalse(task.isCancelled());

            taskLatch.countDown();
    assertEquals(25, task.get().intValue());
    assertTrue(listenerLatch.await(5, TimeUnit.SECONDS));
    assertTrue(task.isDone());
    assertFalse(task.isCancelled());
  }

  public void testListenerCalledOnException() throws Exception {
    throwException = true;

        exec.execute(task);
    runLatch.await();
    taskLatch.countDown();

    try {
      task.get(5, TimeUnit.SECONDS);
      fail("Should have propagated the failure.");
    } catch (ExecutionException e) {
      assertEquals(IllegalStateException.class, e.getCause().getClass());
    }

    assertTrue(listenerLatch.await(5, TimeUnit.SECONDS));
    assertTrue(task.isDone());
    assertFalse(task.isCancelled());
  }

  public void testListenerCalledOnCancelFromNotRunning() throws Exception {
    task.cancel(false);
    assertTrue(task.isDone());
    assertTrue(task.isCancelled());
    assertEquals(1, runLatch.getCount());

        listenerLatch.await(5, TimeUnit.SECONDS);
    assertTrue(task.isDone());
    assertTrue(task.isCancelled());

        assertEquals(1, runLatch.getCount());
  }

  public void testListenerCalledOnCancelFromRunning() throws Exception {
    exec.execute(task);
    runLatch.await();

        task.cancel(true);
    assertTrue(task.isDone());
    assertTrue(task.isCancelled());
    assertEquals(1, taskLatch.getCount());

        listenerLatch.await(5, TimeUnit.SECONDS);
    assertTrue(task.isDone());
    assertTrue(task.isCancelled());
    assertEquals(1, taskLatch.getCount());
  }
}
