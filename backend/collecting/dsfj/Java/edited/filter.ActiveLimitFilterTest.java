
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.support.BlockMyInvoker;
import com.alibaba.dubbo.rpc.support.MockInvocation;
import com.alibaba.dubbo.rpc.support.MyInvoker;

import org.junit.Test;

import java.util.concurrent.CountDownLatch;

import static org.junit.Assert.assertNotSame;


public class ActiveLimitFilterTest {

    private static volatile int count = 0;
    Filter activeLimitFilter = new ActiveLimitFilter();

    @Test
    public void testInvokeNoActives() {
        URL url = URL.valueOf("test:        Invoker<ActiveLimitFilterTest> invoker = new MyInvoker<ActiveLimitFilterTest>(url);
        Invocation invocation = new MockInvocation();
        activeLimitFilter.invoke(invoker, invocation);
    }

    @Test
    public void testInvokeLessActives() {
        URL url = URL.valueOf("test:        Invoker<ActiveLimitFilterTest> invoker = new MyInvoker<ActiveLimitFilterTest>(url);
        Invocation invocation = new MockInvocation();
        activeLimitFilter.invoke(invoker, invocation);
    }

    @Test
    public void testInvokeGreaterActives() {
        URL url = URL.valueOf("test:        final Invoker<ActiveLimitFilterTest> invoker = new BlockMyInvoker<ActiveLimitFilterTest>(url, 100);
        final Invocation invocation = new MockInvocation();
        final CountDownLatch latch = new CountDownLatch(1);
        for (int i = 0; i < 100; i++) {
            Thread thread = new Thread(new Runnable() {

                public void run() {
                    try {
                        latch.await();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    for (int i = 0; i < 100; i++) {
                        try {
                            activeLimitFilter.invoke(invoker, invocation);
                        } catch (RpcException expected) {
                            count++;
                        }
                    }
                }
            });
            thread.start();
        }
        latch.countDown();

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        assertNotSame(0, count);
    }
}