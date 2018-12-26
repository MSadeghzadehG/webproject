
package com.alibaba.dubbo.rpc.protocol.dubbo.telnet;

import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.telnet.TelnetHandler;

import org.easymock.EasyMock;
import org.junit.After;
import org.junit.Test;

import static org.junit.Assert.assertEquals;


public class CurrentTelnetHandlerTest {

    private static TelnetHandler count = new CurrentTelnetHandler();
    private Channel mockChannel;

    @After
    public void after() {
        EasyMock.reset(mockChannel);
    }

    @Test
    public void testService() throws RemotingException {
        mockChannel = EasyMock.createMock(Channel.class);
        EasyMock.expect(mockChannel.getAttribute("telnet.service")).andReturn("com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService").anyTimes();
        EasyMock.replay(mockChannel);
        String result = count.telnet(mockChannel, "");
        assertEquals("com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService", result);
    }

    @Test
    public void testSlash() throws RemotingException {
        mockChannel = EasyMock.createMock(Channel.class);
        EasyMock.expect(mockChannel.getAttribute("telnet.service")).andReturn(null).anyTimes();
        EasyMock.replay(mockChannel);
        String result = count.telnet(mockChannel, "");
        assertEquals("/", result);
    }

    @Test
    public void testMessageError() throws RemotingException {
        mockChannel = EasyMock.createMock(Channel.class);
        EasyMock.expect(mockChannel.getAttribute("telnet.service")).andReturn(null).anyTimes();
        EasyMock.replay(mockChannel);
        String result = count.telnet(mockChannel, "test");
        assertEquals("Unsupported parameter test for pwd.", result);
    }
}