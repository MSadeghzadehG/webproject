
package com.alibaba.dubbo.common.extensionloader.ext6_wrap.impl;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extensionloader.ext6_wrap.WrappedExt;

import java.util.concurrent.atomic.AtomicInteger;

public class Ext5Wrapper1 implements WrappedExt {
    public static AtomicInteger echoCount = new AtomicInteger();
    WrappedExt instance;

    public Ext5Wrapper1(WrappedExt instance) {
        this.instance = instance;
    }

    public String echo(URL url, String s) {
        echoCount.incrementAndGet();
        return instance.echo(url, s);
    }
}