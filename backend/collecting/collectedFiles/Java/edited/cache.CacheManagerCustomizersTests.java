

package org.springframework.boot.autoconfigure.cache;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.junit.Test;

import org.springframework.cache.CacheManager;
import org.springframework.cache.caffeine.CaffeineCacheManager;
import org.springframework.cache.concurrent.ConcurrentMapCacheManager;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;


public class CacheManagerCustomizersTests {

	@Test
	public void customizeWithNullCustomizersShouldDoNothing() {
		new CacheManagerCustomizers(null).customize(mock(CacheManager.class));
	}

	@Test
	public void customizeSimpleCacheManager() {
		CacheManagerCustomizers customizers = new CacheManagerCustomizers(
				Collections.singletonList(new CacheNamesCacheManagerCustomizer()));
		ConcurrentMapCacheManager cacheManager = new ConcurrentMapCacheManager();
		customizers.customize(cacheManager);
		assertThat(cacheManager.getCacheNames()).containsOnly("one", "two");
	}

	@Test
	public void customizeShouldCheckGeneric() {
		List<TestCustomizer<?>> list = new ArrayList<>();
		list.add(new TestCustomizer<>());
		list.add(new TestConcurrentMapCacheManagerCustomizer());
		CacheManagerCustomizers customizers = new CacheManagerCustomizers(list);
		customizers.customize(mock(CacheManager.class));
		assertThat(list.get(0).getCount()).isEqualTo(1);
		assertThat(list.get(1).getCount()).isEqualTo(0);
		customizers.customize(mock(ConcurrentMapCacheManager.class));
		assertThat(list.get(0).getCount()).isEqualTo(2);
		assertThat(list.get(1).getCount()).isEqualTo(1);
		customizers.customize(mock(CaffeineCacheManager.class));
		assertThat(list.get(0).getCount()).isEqualTo(3);
		assertThat(list.get(1).getCount()).isEqualTo(1);
	}

	static class CacheNamesCacheManagerCustomizer
			implements CacheManagerCustomizer<ConcurrentMapCacheManager> {

		@Override
		public void customize(ConcurrentMapCacheManager cacheManager) {
			cacheManager.setCacheNames(Arrays.asList("one", "two"));
		}

	}

	private static class TestCustomizer<T extends CacheManager>
			implements CacheManagerCustomizer<T> {

		private int count;

		@Override
		public void customize(T cacheManager) {
			this.count++;
		}

		public int getCount() {
			return this.count;
		}

	}

	private static class TestConcurrentMapCacheManagerCustomizer
			extends TestCustomizer<ConcurrentMapCacheManager> {

	}

}
