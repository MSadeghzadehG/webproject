
package com.alibaba.dubbo.remoting.handler;

import com.alibaba.dubbo.remoting.ExecutionException;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.Request;
import com.alibaba.dubbo.remoting.exchange.Response;
import com.alibaba.dubbo.remoting.transport.dispatcher.connection.ConnectionOrderedChannelHandler;

import junit.framework.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.atomic.AtomicInteger;


public class ConnectChannelHandlerTest extends WrappedChannelHandlerTest {

    @Before
    public void setUp() throws Exception {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(true), url);
    }

    @Test
    public void test_Connect_Blocked() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(false), url);
        ThreadPoolExecutor executor = (ThreadPoolExecutor) getField(handler, "connectionExecutor", 1);
        Assert.assertEquals(1, executor.getMaximumPoolSize());

        int runs = 20;
        int taskCount = runs * 2;
        for (int i = 0; i < runs; i++) {
            handler.connected(new MockedChannel());
            handler.disconnected(new MockedChannel());
            Assert.assertTrue(executor.getActiveCount() + " must <=1", executor.getActiveCount() <= 1);
        }
                Assert.assertEquals(taskCount - 1, executor.getQueue().size());

        for (int i = 0; i < taskCount; i++) {
            if (executor.getCompletedTaskCount() < taskCount) {
                sleep(100);
            }
        }
        Assert.assertEquals(taskCount, executor.getCompletedTaskCount());
    }

    @Test     public void test_Connect_Biz_Error() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(true), url);
        handler.connected(new MockedChannel());
    }

    @Test     public void test_Disconnect_Biz_Error() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(true), url);
        handler.disconnected(new MockedChannel());
    }

    @Test(expected = ExecutionException.class)
    public void test_Connect_Execute_Error() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(false), url);
        ThreadPoolExecutor executor = (ThreadPoolExecutor) getField(handler, "connectionExecutor", 1);
        executor.shutdown();
        handler.connected(new MockedChannel());
    }

    @Test(expected = ExecutionException.class)
    public void test_Disconnect_Execute_Error() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(false), url);
        ThreadPoolExecutor executor = (ThreadPoolExecutor) getField(handler, "connectionExecutor", 1);
        executor.shutdown();
        handler.disconnected(new MockedChannel());
    }

        @Test    public void test_MessageReceived_Biz_Error() throws RemotingException {
        handler.received(new MockedChannel(), "");
    }

        @Test
    public void test_Caught_Biz_Error() throws RemotingException {
        handler.caught(new MockedChannel(), new BizException());
    }

    @Test(expected = ExecutionException.class)
    public void test_Received_InvokeInExecuter() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(false), url);
        ThreadPoolExecutor executor = (ThreadPoolExecutor) getField(handler, "SHARED_EXECUTOR", 1);
        executor.shutdown();
        executor = (ThreadPoolExecutor) getField(handler, "executor", 1);
        executor.shutdown();
        handler.received(new MockedChannel(), "");
    }

    
    @SuppressWarnings("deprecation")
    @Ignore("Heartbeat is processed in HeartbeatHandler not WrappedChannelHandler.")
    @Test
    public void test_Received_Event_invoke_direct() throws RemotingException {
        handler = new ConnectionOrderedChannelHandler(new BizChannelHander(false), url);
        ThreadPoolExecutor executor = (ThreadPoolExecutor) getField(handler, "SHARED_EXECUTOR", 1);
        executor.shutdown();
        executor = (ThreadPoolExecutor) getField(handler, "executor", 1);
        executor.shutdown();
        Request req = new Request();
        req.setHeartbeat(true);
        final AtomicInteger count = new AtomicInteger(0);
        handler.received(new MockedChannel() {
            @Override
            public void send(Object message) throws RemotingException {
                Assert.assertEquals("response.heartbeat", true, ((Response) message).isHeartbeat());
                count.incrementAndGet();
            }
        }, req);
        Assert.assertEquals("channel.send must be invoke", 1, count.get());
    }
}