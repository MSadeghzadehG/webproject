
package com.iluwatar.threadpool;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.function.IntFunction;
import java.util.stream.Collectors;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTimeout;


public abstract class TaskTest<T extends Task> {

  
  private static final int TASK_COUNT = 128 * 1024;

  
  private static final int THREAD_COUNT = 8;

  
  private final IntFunction<T> factory;

  
  private final int expectedExecutionTime;

  
  public TaskTest(final IntFunction<T> factory, final int expectedExecutionTime) {
    this.factory = factory;
    this.expectedExecutionTime = expectedExecutionTime;
  }

  
  @Test
  public void testIdGeneration() throws Exception {
    assertTimeout(ofMillis(10000), () -> {
      final ExecutorService service = Executors.newFixedThreadPool(THREAD_COUNT);

      final List<Callable<Integer>> tasks = new ArrayList<>();
      for (int i = 0; i < TASK_COUNT; i++) {
        tasks.add(() -> factory.apply(1).getId());
      }

      final List<Integer> ids = service.invokeAll(tasks)
          .stream()
          .map(TaskTest::get)
          .filter(Objects::nonNull)
          .collect(Collectors.toList());

      service.shutdownNow();

      final long uniqueIdCount = ids.stream()
          .distinct()
          .count();

      assertEquals(TASK_COUNT, ids.size());
      assertEquals(TASK_COUNT, uniqueIdCount);
    });
  }

  
  @Test
  public void testTimeMs() {
    for (int i = 0; i < 10; i++) {
      assertEquals(this.expectedExecutionTime * i, this.factory.apply(i).getTimeMs());
    }
  }

  
  @Test
  public void testToString() {
    assertNotNull(this.factory.apply(0).toString());
  }

  
  private static <O> O get(Future<O> future) {
    try {
      return future.get();
    } catch (InterruptedException | ExecutionException e) {
      return null;
    }
  }

}
