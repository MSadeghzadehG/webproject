

package org.springframework.boot.autoconfigure.cache.support;

import java.net.URI;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.cache.Cache;
import javax.cache.CacheManager;
import javax.cache.configuration.Configuration;
import javax.cache.configuration.OptionalFeature;
import javax.cache.spi.CachingProvider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class MockCachingProvider implements CachingProvider {

	@Override
	@SuppressWarnings("rawtypes")
	public CacheManager getCacheManager(URI uri, ClassLoader classLoader,
			Properties properties) {
		CacheManager cacheManager = mock(CacheManager.class);
		given(cacheManager.getURI()).willReturn(uri);
		given(cacheManager.getClassLoader()).willReturn(classLoader);
		final Map<String, Cache> caches = new HashMap<>();
		given(cacheManager.getCacheNames()).willReturn(caches.keySet());
		given(cacheManager.getCache(anyString())).willAnswer((invocation) -> {
			String cacheName = invocation.getArgument(0);
			return caches.get(cacheName);
		});
		given(cacheManager.createCache(anyString(), any(Configuration.class)))
				.will((invocation) -> {
					String cacheName = invocation.getArgument(0);
					Cache cache = mock(Cache.class);
					given(cache.getName()).willReturn(cacheName);
					caches.put(cacheName, cache);
					return cache;
				});
		return cacheManager;
	}

	@Override
	public ClassLoader getDefaultClassLoader() {
		return mock(ClassLoader.class);
	}

	@Override
	public URI getDefaultURI() {
		return null;
	}

	@Override
	public Properties getDefaultProperties() {
		return new Properties();
	}

	@Override
	public CacheManager getCacheManager(URI uri, ClassLoader classLoader) {
		return getCacheManager(uri, classLoader, getDefaultProperties());
	}

	@Override
	public CacheManager getCacheManager() {
		return getCacheManager(getDefaultURI(), getDefaultClassLoader());
	}

	@Override
	public void close() {
	}

	@Override
	public void close(ClassLoader classLoader) {
	}

	@Override
	public void close(URI uri, ClassLoader classLoader) {
	}

	@Override
	public boolean isSupported(OptionalFeature optionalFeature) {
		return false;
	}

}
