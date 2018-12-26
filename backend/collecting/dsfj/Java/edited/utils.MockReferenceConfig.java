
package com.alibaba.dubbo.config.utils;

import com.alibaba.dubbo.config.ReferenceConfig;

import java.util.concurrent.atomic.AtomicLong;

public class MockReferenceConfig extends ReferenceConfig<String> {
    static AtomicLong counter = new AtomicLong();

    String value;
    boolean destroyMethodRun = false;

    public static void setCounter(long c) {
        counter.set(c);
    }

    public boolean isGetMethodRun() {
        return value != null;
    }

    public boolean isDestroyMethodRun() {
        return destroyMethodRun;
    }

    @Override
    public synchronized String get() {
        if (value != null) return value;

        value = "" + counter.getAndIncrement();
        return value;
    }

    @Override
    public synchronized void destroy() {
        destroyMethodRun = true;
    }
}
