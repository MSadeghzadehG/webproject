

package org.springframework.boot.autoconfigure.cache;

import java.util.Properties;

import javax.cache.CacheManager;
import javax.cache.spi.CachingProvider;


interface JCachePropertiesCustomizer {

	
	void customize(CacheProperties cacheProperties, Properties properties);

}
