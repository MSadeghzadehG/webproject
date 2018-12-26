
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcResult;
import com.alibaba.dubbo.rpc.support.DemoService;

import org.easymock.EasyMock;
import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class EchoFilterTest {

    Filter echoFilter = new EchoFilter();

    @SuppressWarnings("unchecked")
    @Test
    public void testEcho() {
        Invocation invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("$echo").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Enum.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.expect(invocation.getAttachments()).andReturn(null).anyTimes();
        EasyMock.replay(invocation);
        Invoker<DemoService> invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = echoFilter.invoke(invoker, invocation);
        assertEquals("hello", filterResult.getValue());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void testNonEcho() {
        Invocation invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("echo").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Enum.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.expect(invocation.getAttachments()).andReturn(null).anyTimes();
        EasyMock.replay(invocation);
        Invoker<DemoService> invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = echoFilter.invoke(invoker, invocation);
        assertEquals("High", filterResult.getValue());
    }
}