
package com.alibaba.dubbo.rpc.cluster.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.LogUtil;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcContext;
import com.alibaba.dubbo.rpc.RpcInvocation;
import com.alibaba.dubbo.rpc.RpcResult;
import com.alibaba.dubbo.rpc.cluster.Directory;

import junit.framework.Assert;
import org.easymock.EasyMock;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;


@SuppressWarnings("unchecked")
public class FailbackClusterInvokerTest {

    List<Invoker<FailbackClusterInvokerTest>> invokers = new ArrayList<Invoker<FailbackClusterInvokerTest>>();
    URL url = URL.valueOf("test:    Invoker<FailbackClusterInvokerTest> invoker = EasyMock.createMock(Invoker.class);
    RpcInvocation invocation = new RpcInvocation();
    Directory<FailbackClusterInvokerTest> dic;
    Result result = new RpcResult();

    

    @Before
    public void setUp() throws Exception {

        dic = EasyMock.createMock(Directory.class);
        EasyMock.expect(dic.getUrl()).andReturn(url).anyTimes();
        EasyMock.expect(dic.list(invocation)).andReturn(invokers).anyTimes();
        EasyMock.expect(dic.getInterface()).andReturn(FailbackClusterInvokerTest.class).anyTimes();

        invocation.setMethodName("method1");
        EasyMock.replay(dic);

        invokers.add(invoker);
    }

    @After
    public void tearDown() {
        EasyMock.verify(invoker, dic);

    }

    private void resetInvokerToException() {
        EasyMock.reset(invoker);
        EasyMock.expect(invoker.invoke(invocation)).andThrow(new RuntimeException()).anyTimes();
        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(FailbackClusterInvokerTest.class).anyTimes();
        EasyMock.replay(invoker);
    }

    private void resetInvokerToNoException() {
        EasyMock.reset(invoker);
        EasyMock.expect(invoker.invoke(invocation)).andReturn(result).anyTimes();
        EasyMock.expect(invoker.getUrl()).andReturn(url).anyTimes();
        EasyMock.expect(invoker.getInterface()).andReturn(FailbackClusterInvokerTest.class).anyTimes();
        EasyMock.replay(invoker);
    }

    @Test
    public void testInvokeExceptoin() {
        resetInvokerToException();
        FailbackClusterInvoker<FailbackClusterInvokerTest> invoker = new FailbackClusterInvoker<FailbackClusterInvokerTest>(
                dic);
        invoker.invoke(invocation);
        Assert.assertNull(RpcContext.getContext().getInvoker());
    }

    @Test()
    public void testInvokeNoExceptoin() {

        resetInvokerToNoException();

        FailbackClusterInvoker<FailbackClusterInvokerTest> invoker = new FailbackClusterInvoker<FailbackClusterInvokerTest>(
                dic);
        Result ret = invoker.invoke(invocation);
        Assert.assertSame(result, ret);
    }

    @Test()
    public void testNoInvoke() {
        dic = EasyMock.createMock(Directory.class);

        EasyMock.expect(dic.getUrl()).andReturn(url).anyTimes();
        EasyMock.expect(dic.list(invocation)).andReturn(null).anyTimes();
        EasyMock.expect(dic.getInterface()).andReturn(FailbackClusterInvokerTest.class).anyTimes();

        invocation.setMethodName("method1");
        EasyMock.replay(dic);

        invokers.add(invoker);

        resetInvokerToNoException();

        FailbackClusterInvoker<FailbackClusterInvokerTest> invoker = new FailbackClusterInvoker<FailbackClusterInvokerTest>(
                dic);
        LogUtil.start();
        invoker.invoke(invocation);
        assertEquals(1, LogUtil.findMessage("Failback to invoke"));
        LogUtil.stop();
    }

    @Test()
    public void testRetryFailed() {

        resetInvokerToException();

        FailbackClusterInvoker<FailbackClusterInvokerTest> invoker = new FailbackClusterInvoker<FailbackClusterInvokerTest>(
                dic);
        invoker.invoke(invocation);
        Assert.assertNull(RpcContext.getContext().getInvoker());
        invoker.retryFailed();            }
}