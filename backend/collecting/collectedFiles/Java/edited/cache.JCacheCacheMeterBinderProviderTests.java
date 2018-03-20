

package org.springframework.boot.actuate.metrics.cache;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Collections;

import io.micrometer.core.instrument.binder.MeterBinder;
import io.micrometer.core.instrument.binder.cache.JCacheMetrics;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import org.springframework.cache.jcache.JCacheCache;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


@RunWith(MockitoJUnitRunner.class)
public class JCacheCacheMeterBinderProviderTests {

	@Mock
	private javax.cache.Cache<Object, Object> nativeCache;

	@Test
	public void jCacheCacheProvider() throws URISyntaxException {
		javax.cache.CacheManager cacheManager = mock(javax.cache.CacheManager.class);
		given(cacheManager.getURI()).willReturn(new URI("/test"));
		given(this.nativeCache.getCacheManager()).willReturn(cacheManager);
		given(this.nativeCache.getName()).willReturn("test");
		JCacheCache cache = new JCacheCache(this.nativeCache);
		MeterBinder meterBinder = new JCacheCacheMeterBinderProvider()
				.getMeterBinder(cache, Collections.emptyList());
		assertThat(meterBinder).isInstanceOf(JCacheMetrics.class);
	}

}
