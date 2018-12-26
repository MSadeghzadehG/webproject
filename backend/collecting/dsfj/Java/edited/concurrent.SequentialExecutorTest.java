

package com.google.common.util.concurrent;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.util.concurrent.Uninterruptibles.awaitUninterruptibly;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.common.collect.Queues;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import junit.framework.TestCase;


public class SequentialExecutorTest extends TestCase {

  private static class FakeExecutor implements Executor {
    Queue<Runnable> tasks = Queues.newArrayDeque();

    @Override
    public void execute(Runnable command) {
      tasks.add(command);
    }

    boolean hasNext() {
      return !tasks.isEmpty();
    }

    void runNext() {
      assertTrue("expected at least one task to run", hasNext());
      tasks.remove().run();
    }

    void runAll() {
      while (hasNext()) {
        runNext();
      }
    }
  }

  private FakeExecutor fakePool;
  private SequentialExecutor e;

  @Override
  public void setUp() {
    fakePool = new FakeExecutor();
    e = new SequentialExecutor(fakePool);
  }

  public void testConstructingWithNullExecutor_fails() {
    try {
      new SequentialExecutor(null);
      fail("Should have failed with NullPointerException.");
    } catch (NullPointerException expected) {
    }
  }

  public void testBasics() {
    final AtomicInteger totalCalls = new AtomicInteger();
    Runnable intCounter =
        new Runnable() {
          @Override
          public void run() {
            totalCalls.incrementAndGet();
                        assertFalse(fakePool.hasNext());
          }
        };

    assertFalse(fakePool.hasNext());
    e.execute(intCounter);
        assertTrue(fakePool.hasNext());
    e.execute(intCounter);
        assertEquals(0, totalCalls.get());
    fakePool.runAll();
    assertEquals(2, totalCalls.get());
        assertFalse(fakePool.hasNext());

        e.execute(intCounter);
    e.execute(intCounter);
    e.execute(intCounter);
        assertEquals(2, totalCalls.get());
    fakePool.runAll();
    assertEquals(5, totalCalls.get());
    assertFalse(fakePool.hasNext());
  }

  public void testOrdering() {
    final List<Integer> callOrder = Lists.newArrayList();

    class FakeOp implements Runnable {
      final int op;

      FakeOp(int op) {
        this.op = op;
      }

      @Override
      public void run() {
        callOrder.add(op);
      }
    }

    e.execute(new FakeOp(0));
    e.execute(new FakeOp(1));
    e.execute(new FakeOp(2));
    fakePool.runAll();

    assertEquals(ImmutableList.of(0, 1, 2), callOrder);
  }

  public void testRuntimeException_doesNotStopExecution() {

    final AtomicInteger numCalls = new AtomicInteger();

    Runnable runMe =
        new Runnable() {
          @Override
          public void run() {
            numCalls.incrementAndGet();
            throw new RuntimeException("FAKE EXCEPTION!");
          }
        };

    e.execute(runMe);
    e.execute(runMe);
    fakePool.runAll();

    assertEquals(2, numCalls.get());
  }

  public void testInterrupt_beforeRunRestoresInterruption() throws Exception {
        fakePool.execute(
        new Runnable() {
          @Override
          public void run() {
            Thread.currentThread().interrupt();
          }
        });
        e.execute(
        new Runnable() {
          @Override
          public void run() {
            assertThat(Thread.currentThread().isInterrupted()).isFalse();
          }
        });

        fakePool.runAll();

                assertThat(Thread.currentThread().interrupted()).isTrue();
  }

  public void testInterrupt_doesNotInterruptSubsequentTask() throws Exception {
        e.execute(
        new Runnable() {
          @Override
          public void run() {
            Thread.currentThread().interrupt();
          }
        });
        e.execute(
        new Runnable() {
          @Override
          public void run() {
            assertThat(Thread.currentThread().isInterrupted()).isFalse();
          }
        });

        fakePool.runAll();

                assertThat(Thread.currentThread().interrupted()).isTrue();
  }

  public void testInterrupt_doesNotStopExecution() {

    final AtomicInteger numCalls = new AtomicInteger();

    Runnable runMe =
        new Runnable() {
          @Override
          public void run() {
            numCalls.incrementAndGet();
          }
        };

    Thread.currentThread().interrupt();

    e.execute(runMe);
    e.execute(runMe);
    fakePool.runAll();

    assertEquals(2, numCalls.get());

    assertTrue(Thread.interrupted());
  }

  public void testDelegateRejection() {
    final AtomicInteger numCalls = new AtomicInteger();
    final AtomicBoolean reject = new AtomicBoolean(true);
    final SequentialExecutor executor =
        new SequentialExecutor(
            new Executor() {
              @Override
              public void execute(Runnable r) {
                if (reject.get()) {
                  throw new RejectedExecutionException();
                }
                r.run();
              }
            });
    Runnable task =
        new Runnable() {
          @Override
          public void run() {
            numCalls.incrementAndGet();
          }
        };
    try {
      executor.execute(task);
      fail();
    } catch (RejectedExecutionException expected) {
    }
    assertEquals(0, numCalls.get());
    reject.set(false);
    executor.execute(task);
    assertEquals(1, numCalls.get());
  }

  public void testTaskThrowsError() throws Exception {
    class MyError extends Error {}
    final CyclicBarrier barrier = new CyclicBarrier(2);
        ExecutorService service = Executors.newSingleThreadExecutor();
    try {
      final SequentialExecutor executor = new SequentialExecutor(service);
      Runnable errorTask =
          new Runnable() {
            @Override
            public void run() {
              throw new MyError();
            }
          };
      Runnable barrierTask =
          new Runnable() {
            @Override
            public void run() {
              try {
                barrier.await();
              } catch (Exception e) {
                throw new RuntimeException(e);
              }
            }
          };
      executor.execute(errorTask);
      service.execute(barrierTask);                   barrier.await(1, TimeUnit.SECONDS);
      executor.execute(barrierTask);
            barrier.await(1, TimeUnit.SECONDS);
    } finally {
      service.shutdown();
    }
  }

  public void testRejectedExecutionThrownWithMultipleCalls() throws Exception {
    final CountDownLatch latch = new CountDownLatch(1);
    final SettableFuture<?> future = SettableFuture.create();
    final Executor delegate =
        new Executor() {
          @Override
          public void execute(Runnable task) {
            if (future.set(null)) {
              awaitUninterruptibly(latch);
            }
            throw new RejectedExecutionException();
          }
        };
    final SequentialExecutor executor = new SequentialExecutor(delegate);
    final ExecutorService blocked = Executors.newCachedThreadPool();
    Future<?> first =
        blocked.submit(
            new Runnable() {
              @Override
              public void run() {
                executor.execute(Runnables.doNothing());
              }
            });
    future.get(10, TimeUnit.SECONDS);
    try {
      executor.execute(Runnables.doNothing());
      fail();
    } catch (RejectedExecutionException expected) {
    }
    latch.countDown();
    try {
      first.get(10, TimeUnit.SECONDS);
      fail();
    } catch (ExecutionException expected) {
      assertThat(expected).hasCauseThat().isInstanceOf(RejectedExecutionException.class);
    }
  }
}
