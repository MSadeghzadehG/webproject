
package com.alibaba.dubbo.rpc.protocol.dubbo;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.RpcInvocation;
import com.alibaba.dubbo.rpc.RpcResult;
import com.alibaba.dubbo.rpc.protocol.dubbo.filter.FutureFilter;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService;

import org.easymock.EasyMock;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class FutureFilterTest {
    private static RpcInvocation invocation;
    Filter eventFilter = new FutureFilter();

    @BeforeClass
    public static void setUp() {
        invocation = new RpcInvocation();
        invocation.setMethodName("echo");
        invocation.setParameterTypes(new Class<?>[]{Enum.class});
        invocation.setArguments(new Object[]{"hello"});
    }

    @Test
    public void testSyncCallback() {
        @SuppressWarnings("unchecked")
        Invoker<DemoService> invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = eventFilter.invoke(invoker, invocation);
        assertEquals("High", filterResult.getValue());
    }

    @Test(expected = RuntimeException.class)
    public void testSyncCallbackHasException() throws RpcException, Throwable {
        @SuppressWarnings("unchecked")
        Invoker<DemoService> invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setException(new RuntimeException());
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        eventFilter.invoke(invoker, invocation).recreate();
    }
}