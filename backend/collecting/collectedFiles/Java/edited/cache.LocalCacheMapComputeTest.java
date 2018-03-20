

package com.google.common.cache;

import static com.google.common.truth.Truth.assertThat;

import java.util.concurrent.TimeUnit;
import java.util.function.IntConsumer;
import java.util.stream.IntStream;
import junit.framework.TestCase;


public class LocalCacheMapComputeTest extends TestCase {
  final int count = 10000;
  final String delimiter = "-";
  final String key = "key";
  Cache<String, String> cache;

    private static void doParallelCacheOp(int count, IntConsumer consumer) {
    IntStream.range(0, count).parallel().forEach(consumer);
  }

  @Override
  public void setUp() throws Exception {
    super.setUp();
    this.cache =
        CacheBuilder.newBuilder()
            .expireAfterAccess(500000, TimeUnit.MILLISECONDS)
            .maximumSize(count)
            .build();
  }

  public void testComputeIfAbsent() {
        doParallelCacheOp(
        count,
        n -> {
          cache.asMap().computeIfAbsent(key, k -> "value" + n);
        });
    assertEquals(1, cache.size());
  }

  public void testComputeIfPresent() {
    cache.put(key, "1");
        doParallelCacheOp(
        count,
        n -> {
          cache.asMap().computeIfPresent(key, (k, v) -> v + delimiter + n);
        });
    assertEquals(1, cache.size());
    assertThat(cache.getIfPresent(key).split(delimiter)).hasLength(count + 1);
  }

  public void testUpdates() {
    cache.put(key, "1");
        doParallelCacheOp(
        count,
        n -> {
          cache.asMap().compute(key, (k, v) -> n % 2 == 0 ? v + delimiter + n : null);
        });
    assertTrue(1 >= cache.size());
  }

  public void testCompute() {
    cache.put(key, "1");
        doParallelCacheOp(
        count,
        n -> {
          cache.asMap().compute(key, (k, v) -> null);
        });
    assertEquals(0, cache.size());
  }
}
