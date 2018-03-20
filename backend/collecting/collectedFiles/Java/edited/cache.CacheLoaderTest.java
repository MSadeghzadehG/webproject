

package com.google.common.cache;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import java.util.LinkedList;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicInteger;
import junit.framework.TestCase;


public class CacheLoaderTest extends TestCase {

  private static class QueuingExecutor implements Executor {
    private LinkedList<Runnable> tasks = Lists.newLinkedList();

    @Override
    public void execute(Runnable task) {
      tasks.add(task);
    }

    private void runNext() {
      tasks.removeFirst().run();
    }
  }

  public void testAsyncReload() throws Exception {
    final AtomicInteger loadCount = new AtomicInteger();
    final AtomicInteger reloadCount = new AtomicInteger();
    final AtomicInteger loadAllCount = new AtomicInteger();

    CacheLoader<Object, Object> baseLoader =
        new CacheLoader<Object, Object>() {
          @Override
          public Object load(Object key) {
            loadCount.incrementAndGet();
            return new Object();
          }

          @Override
          public ListenableFuture<Object> reload(Object key, Object oldValue) {
            reloadCount.incrementAndGet();
            return Futures.immediateFuture(new Object());
          }

          @Override
          public Map<Object, Object> loadAll(Iterable<?> keys) {
            loadAllCount.incrementAndGet();
            return ImmutableMap.of();
          }
        };

    assertEquals(0, loadCount.get());
    assertEquals(0, reloadCount.get());
    assertEquals(0, loadAllCount.get());

    baseLoader.load(new Object());
    @SuppressWarnings("unused")     Future<?> possiblyIgnoredError = baseLoader.reload(new Object(), new Object());
    baseLoader.loadAll(ImmutableList.of(new Object()));
    assertEquals(1, loadCount.get());
    assertEquals(1, reloadCount.get());
    assertEquals(1, loadAllCount.get());

    QueuingExecutor executor = new QueuingExecutor();
    CacheLoader<Object, Object> asyncReloader = CacheLoader.asyncReloading(baseLoader, executor);

    asyncReloader.load(new Object());
    @SuppressWarnings("unused")     Future<?> possiblyIgnoredError1 = asyncReloader.reload(new Object(), new Object());
    asyncReloader.loadAll(ImmutableList.of(new Object()));
    assertEquals(2, loadCount.get());
    assertEquals(1, reloadCount.get());
    assertEquals(2, loadAllCount.get());

    executor.runNext();
    assertEquals(2, loadCount.get());
    assertEquals(2, reloadCount.get());
    assertEquals(2, loadAllCount.get());
  }
}
