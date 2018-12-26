
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.LogUtil;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.support.MockInvocation;
import com.alibaba.dubbo.rpc.support.MyInvoker;

import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class AccessLogFilterTest {

    Filter accessLogFilter = new AccessLogFilter();

        @Test
    public void testInvokeException() {
        Invoker<AccessLogFilterTest> invoker = new MyInvoker<AccessLogFilterTest>(null);
        Invocation invocation = new MockInvocation();
        LogUtil.start();
        accessLogFilter.invoke(invoker, invocation);
        assertEquals(1, LogUtil.findMessage("Exception in AcessLogFilter of service"));
        LogUtil.stop();
    }

        @Test
    public void testDefault() {
        URL url = URL.valueOf("test:        Invoker<AccessLogFilterTest> invoker = new MyInvoker<AccessLogFilterTest>(url);
        Invocation invocation = new MockInvocation();
        accessLogFilter.invoke(invoker, invocation);
    }

    @Test
    public void testCustom() {
        URL url = URL.valueOf("test:        Invoker<AccessLogFilterTest> invoker = new MyInvoker<AccessLogFilterTest>(url);
        Invocation invocation = new MockInvocation();
        accessLogFilter.invoke(invoker, invocation);
    }

}