
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.RpcContext;
import com.alibaba.dubbo.rpc.support.DemoService;
import com.alibaba.dubbo.rpc.support.MockInvocation;
import com.alibaba.dubbo.rpc.support.MyInvoker;

import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class ConsumerContextFilterTest {
    Filter consumerContextFilter = new ConsumerContextFilter();

    @Test
    public void testSetContext() {
        URL url = URL.valueOf("test:        Invoker<DemoService> invoker = new MyInvoker<DemoService>(url);
        Invocation invocation = new MockInvocation();
        consumerContextFilter.invoke(invoker, invocation);
        assertEquals(invoker, RpcContext.getContext().getInvoker());
        assertEquals(invocation, RpcContext.getContext().getInvocation());
        assertEquals(NetUtils.getLocalHost() + ":0", RpcContext.getContext().getLocalAddressString());
        assertEquals("test:11", RpcContext.getContext().getRemoteAddressString());

    }
}