
package com.alibaba.dubbo.rpc.filter;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Filter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcResult;
import com.alibaba.dubbo.rpc.support.DemoService;
import com.alibaba.dubbo.rpc.support.Type;

import org.easymock.EasyMock;
import org.junit.After;
import org.junit.Test;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;


public class CompatibleFilterFilterTest {

    Filter compatibleFilter = new CompatibleFilter();
    Invocation invocation;
    Invoker<DemoService> invoker;

    @After
    public void tearDown() {
        EasyMock.reset(invocation, invoker);
    }

    @Test
    public void testInvokerGeneric() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("$enumlength").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Enum.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertEquals(filterResult, result);
    }

    @Test
    public void testResulthasException() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("enumlength").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Enum.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setException(new RuntimeException());
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertEquals(filterResult, result);
    }

    @Test
    public void testInvokerJsonPojoSerialization() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("enumlength").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Type[].class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertEquals(Type.High, filterResult.getValue());
    }

    @Test
    public void testInvokerNonJsonEnumSerialization() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("enumlength").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{Type[].class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("High");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertEquals(Type.High, filterResult.getValue());
    }

    @Test
    public void testInvokerNonJsonNonPojoSerialization() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("echo").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{String.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue(new String[]{"High"});
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertArrayEquals(new String[]{"High"}, (String[]) filterResult.getValue());
    }

    @Test
    public void testInvokerNonJsonPojoSerialization() {
        invocation = EasyMock.createMock(Invocation.class);
        EasyMock.expect(invocation.getMethodName()).andReturn("echo").anyTimes();
        EasyMock.expect(invocation.getParameterTypes()).andReturn(new Class<?>[]{String.class}).anyTimes();
        EasyMock.expect(invocation.getArguments()).andReturn(new Object[]{"hello"}).anyTimes();
        EasyMock.replay(invocation);
        invoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(invoker.isAvailable()).andReturn(true).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(DemoService.class).anyTimes();
        RpcResult result = new RpcResult();
        result.setValue("hello");
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        URL url = URL.valueOf("test:        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.replay(invoker);
        Result filterResult = compatibleFilter.invoke(invoker, invocation);
        assertEquals("hello", filterResult.getValue());
    }
}