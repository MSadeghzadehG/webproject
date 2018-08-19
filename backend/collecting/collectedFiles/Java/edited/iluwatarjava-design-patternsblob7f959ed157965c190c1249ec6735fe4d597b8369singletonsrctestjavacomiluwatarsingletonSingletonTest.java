
package com.iluwatar.singleton;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.function.Supplier;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTimeout;


public abstract class SingletonTest<S> {

  
  private final Supplier<S> singletonInstanceMethod;

  
  public SingletonTest(final Supplier<S> singletonInstanceMethod) {
    this.singletonInstanceMethod = singletonInstanceMethod;
  }

  
  @Test
  public void testMultipleCallsReturnTheSameObjectInSameThread() {
        S instance1 = this.singletonInstanceMethod.get();
    S instance2 = this.singletonInstanceMethod.get();
    S instance3 = this.singletonInstanceMethod.get();
        assertSame(instance1, instance2);
    assertSame(instance1, instance3);
    assertSame(instance2, instance3);
  }

  
  @Test
  public void testMultipleCallsReturnTheSameObjectInDifferentThreads() throws Exception {
    assertTimeout(ofMillis(10000), () -> {
            final List<Callable<S>> tasks = new ArrayList<>();
      for (int i = 0; i < 10000; i++) {
        tasks.add(this.singletonInstanceMethod::get);
      }

            final ExecutorService executorService = Executors.newFixedThreadPool(8);
      final List<Future<S>> results = executorService.invokeAll(tasks);

            final S expectedInstance = this.singletonInstanceMethod.get();
      for (Future<S> res : results) {
        final S instance = res.get();
        assertNotNull(instance);
        assertSame(expectedInstance, instance);
      }

            executorService.shutdown();
    });

  }

}
