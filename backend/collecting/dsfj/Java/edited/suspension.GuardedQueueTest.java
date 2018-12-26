
package com.iluwatar.guarded.suspension;

import org.junit.jupiter.api.Test;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class GuardedQueueTest {
  private volatile Integer value;

  @Test
  public void testGet() {
    GuardedQueue g = new GuardedQueue();
    ExecutorService executorService = Executors.newFixedThreadPool(2);
    executorService.submit(() -> value = g.get());
    executorService.submit(() -> g.put(Integer.valueOf(10)));
    executorService.shutdown();
    try {
      executorService.awaitTermination(30, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    assertEquals(Integer.valueOf(10), value);
  }

  @Test
  public void testPut() {
    GuardedQueue g = new GuardedQueue();
    g.put(12);
    assertEquals(Integer.valueOf(12), g.get());
  }

}
