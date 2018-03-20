
package com.alibaba.dubbo.config.cache;

import java.util.concurrent.atomic.AtomicInteger;


public class CacheServiceImpl implements CacheService {

    private final AtomicInteger i = new AtomicInteger();

    public String findCache(String id) {
        return "request: " + id + ", response: " + i.getAndIncrement();
    }

}
