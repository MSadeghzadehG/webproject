
package com.alibaba.dubbo.rpc.protocol.dubbo.telnet;

import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.telnet.TelnetHandler;

import org.easymock.EasyMock;
import org.junit.Test;

import static org.junit.Assert.assertTrue;


public class LogTelnetHandlerTest {

    private static TelnetHandler log = new LogTelnetHandler();
    private Channel mockChannel;

    @Test
    public void testChangeLogLevel() throws RemotingException {
        mockChannel = EasyMock.createMock(Channel.class);
        EasyMock.replay(mockChannel);
        String result = log.telnet(mockChannel, "error");
        assertTrue(result.contains("\r\nCURRENT LOG LEVEL:ERROR"));
        String result2 = log.telnet(mockChannel, "warn");
        assertTrue(result2.contains("\r\nCURRENT LOG LEVEL:WARN"));
        EasyMock.reset(mockChannel);
    }

    @Test
    public void testPrintLog() throws RemotingException {
        mockChannel = EasyMock.createMock(Channel.class);
        EasyMock.replay(mockChannel);
        String result = log.telnet(mockChannel, "100");
        assertTrue(result.contains("CURRENT LOG APPENDER"));
        EasyMock.reset(mockChannel);
    }

}