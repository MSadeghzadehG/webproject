

package org.springframework.boot.autoconfigure.cache;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import com.couchbase.client.spring.cache.CouchbaseCacheManager;
import com.hazelcast.spring.cache.HazelcastCacheManager;
import org.infinispan.spring.provider.SpringEmbeddedCacheManager;

import org.springframework.boot.autoconfigure.AutoConfigurations;
import org.springframework.boot.test.context.assertj.AssertableApplicationContext;
import org.springframework.boot.test.context.runner.ApplicationContextRunner;
import org.springframework.boot.test.context.runner.ContextConsumer;
import org.springframework.cache.CacheManager;
import org.springframework.cache.caffeine.CaffeineCacheManager;
import org.springframework.cache.concurrent.ConcurrentMapCacheManager;
import org.springframework.cache.ehcache.EhCacheCacheManager;
import org.springframework.cache.support.SimpleCacheManager;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.redis.cache.RedisCacheManager;

import static org.assertj.core.api.Assertions.assertThat;


abstract class AbstractCacheAutoConfigurationTests {

	protected final ApplicationContextRunner contextRunner = new ApplicationContextRunner()
			.withConfiguration(AutoConfigurations.of(CacheAutoConfiguration.class));

	protected <T extends CacheManager> T getCacheManager(
			AssertableApplicationContext loaded, Class<T> type) {
		CacheManager cacheManager = loaded.getBean(CacheManager.class);
		assertThat(cacheManager).as("Wrong cache manager type").isInstanceOf(type);
		return type.cast(cacheManager);
	}

	@SuppressWarnings("rawtypes")
	protected ContextConsumer<AssertableApplicationContext> verifyCustomizers(
			String... expectedCustomizerNames) {
		return (context) -> {
			CacheManager cacheManager = getCacheManager(context, CacheManager.class);
			List<String> expected = new ArrayList<>(
					Arrays.asList(expectedCustomizerNames));
			Map<String, CacheManagerTestCustomizer> customizer = context
					.getBeansOfType(CacheManagerTestCustomizer.class);
			customizer.forEach((key, value) -> {
				if (expected.contains(key)) {
					expected.remove(key);
					assertThat(value.cacheManager).isSameAs(cacheManager);
				}
				else {
					assertThat(value.cacheManager).isNull();
				}
			});
			assertThat(expected).hasSize(0);
		};
	}

	@Configuration
	static class CacheManagerCustomizersConfiguration {

		@Bean
		public CacheManagerCustomizer<CacheManager> allCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<CacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<ConcurrentMapCacheManager> simpleCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<ConcurrentMapCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<SimpleCacheManager> genericCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<SimpleCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<CouchbaseCacheManager> couchbaseCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<CouchbaseCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<RedisCacheManager> redisCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<RedisCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<EhCacheCacheManager> ehcacheCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<EhCacheCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<HazelcastCacheManager> hazelcastCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<HazelcastCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<SpringEmbeddedCacheManager> infinispanCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<SpringEmbeddedCacheManager>() {

			};
		}

		@Bean
		public CacheManagerCustomizer<CaffeineCacheManager> caffeineCacheManagerCustomizer() {
			return new CacheManagerTestCustomizer<CaffeineCacheManager>() {

			};
		}

	}

	abstract static class CacheManagerTestCustomizer<T extends CacheManager>
			implements CacheManagerCustomizer<T> {

		T cacheManager;

		@Override
		public void customize(T cacheManager) {
			if (this.cacheManager != null) {
				throw new IllegalStateException("Customized invoked twice");
			}
			this.cacheManager = cacheManager;
		}

	}

}
