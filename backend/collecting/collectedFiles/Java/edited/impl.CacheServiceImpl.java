
package com.alibaba.dubbo.examples.cache.impl;

import com.alibaba.dubbo.examples.cache.api.CacheService;

import java.util.concurrent.atomic.AtomicInteger;


public class CacheServiceImpl implements CacheService {

    private final AtomicInteger i = new AtomicInteger();

    public String findCache(String id) {
        return "request: " + id + ", response: " + i.getAndIncrement();
    }

}
