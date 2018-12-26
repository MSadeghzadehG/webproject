
package com.alibaba.dubbo.cache.support.jcache;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;

import javax.cache.Cache;
import javax.cache.CacheException;
import javax.cache.CacheManager;
import javax.cache.Caching;
import javax.cache.configuration.MutableConfiguration;
import javax.cache.expiry.CreatedExpiryPolicy;
import javax.cache.expiry.Duration;
import javax.cache.spi.CachingProvider;
import java.util.concurrent.TimeUnit;


public class JCache implements com.alibaba.dubbo.cache.Cache {

    private final Cache<Object, Object> store;

    public JCache(URL url) {
        String method = url.getParameter(Constants.METHOD_KEY, "");
        String key = url.getAddress() + "." + url.getServiceKey() + "." + method;
                String type = url.getParameter("jcache");

        CachingProvider provider = type == null || type.length() == 0 ? Caching.getCachingProvider() : Caching.getCachingProvider(type);
        CacheManager cacheManager = provider.getCacheManager();
        Cache<Object, Object> cache = cacheManager.getCache(key);
        if (cache == null) {
            try {
                                MutableConfiguration config =
                        new MutableConfiguration<Object, Object>()
                                .setTypes(Object.class, Object.class)
                                .setExpiryPolicyFactory(CreatedExpiryPolicy.factoryOf(new Duration(TimeUnit.MILLISECONDS, url.getMethodParameter(method, "cache.write.expire", 60 * 1000))))
                                .setStoreByValue(false)
                                .setManagementEnabled(true)
                                .setStatisticsEnabled(true);
                cache = cacheManager.createCache(key, config);
            } catch (CacheException e) {
                                cache = cacheManager.getCache(key);
            }
        }

        this.store = cache;
    }

    public void put(Object key, Object value) {
        store.put(key, value);
    }

    public Object get(Object key) {
        return store.get(key);
    }

}
