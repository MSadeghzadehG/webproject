
package com.alibaba.dubbo.rpc.protocol.dubbo.telnet;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.remoting.exchange.Exchangers;
import com.alibaba.dubbo.remoting.telnet.TelnetHandler;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.protocol.dubbo.DubboProtocol;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.ProtocolUtils;

import org.easymock.EasyMock;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;


public class PortTelnetHandlerTest {

    private static TelnetHandler port = new PortTelnetHandler();
    private Invoker<DemoService> mockInvoker;

    @SuppressWarnings("unchecked")
    @Before
    public void before() {
        mockInvoker = EasyMock.createMock(Invoker.class);
        EasyMock.expect(mockInvoker.getInterface()).andReturn(DemoService.class).anyTimes();
        EasyMock.expect(mockInvoker.getUrl()).andReturn(URL.valueOf("dubbo:        EasyMock.replay(mockInvoker);
        DubboProtocol.getDubboProtocol().export(mockInvoker);
    }

    @After
    public void after() {
        EasyMock.reset(mockInvoker);
        ProtocolUtils.closeAll();
    }

    
    @Test
    public void testListClient() throws Exception {
        ExchangeClient client1 = Exchangers.connect("dubbo:        ExchangeClient client2 = Exchangers.connect("dubbo:        Thread.sleep(5000);
        String result = port.telnet(null, "-l 20887");
        String client1Addr = client1.getLocalAddress().toString();
        String client2Addr = client2.getLocalAddress().toString();
        System.out.printf("Result: %s %n", result);
        System.out.printf("Client 1 Address %s %n", client1Addr);
        System.out.printf("Client 2 Address %s %n", client2Addr);
        assertTrue(result.contains(String.valueOf(client1.getLocalAddress().getPort())));
        assertTrue(result.contains(String.valueOf(client2.getLocalAddress().getPort())));

    }

    @Test
    public void testListDetail() throws RemotingException {
        String result = port.telnet(null, "-l");
        assertEquals("dubbo:    }

    @Test
    public void testListAllPort() throws RemotingException {
        String result = port.telnet(null, "");
        assertEquals("20887", result);
    }

    @Test
    public void testErrorMessage() throws RemotingException {
        String result = port.telnet(null, "a");
        assertEquals("Illegal port a, must be integer.", result);
    }

    @Test
    public void testNoPort() throws RemotingException {
        String result = port.telnet(null, "-l 20880");
        assertEquals("No such port 20880", result);
    }

}