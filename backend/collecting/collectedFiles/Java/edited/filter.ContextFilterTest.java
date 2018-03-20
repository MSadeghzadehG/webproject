
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcContext;
import com.alibaba.dubbo.rpc.RpcResult;
import com.alibaba.dubbo.rpc.support.DemoService;
import com.alibaba.dubbo.rpc.support.MockInvocation;
import com.alibaba.dubbo.rpc.support.MyInvoker;

import org.easymock.EasyMock;
import org.junit.Test;

import static org.junit.Assert.assertNull;


public class ContextFilterTest {

    Filter contextFilter = new ContextFilter();
    Invoker<DemoService> invoker;
    Invocation invocation;

    @SuppressWarnings("unchecked")
    @Test
    public void testSetContext() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("$enumlength").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Enum.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.expect(invocation.getAttachments()).andReturn(null).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        contextFilter.invoke(invoker, invocation);
        assertNull(RpcContext.getContext().getInvoker());
    }

    @Test
    public void testWithAttachments() {
        URL url = URL.valueOf("test:        Invoker<DemoService> invoker = new MyInvoker<DemoService>(url);
        Invocation invocation = new MockInvocation();
        Result result = contextFilter.invoke(invoker, invocation);
        assertNull(RpcContext.getContext().getInvoker());
    }
}