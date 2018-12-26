
package com.alibaba.dubbo.remoting.transport.netty;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.Server;
import com.alibaba.dubbo.remoting.exchange.ExchangeChannel;
import com.alibaba.dubbo.remoting.exchange.Exchangers;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;


public class NettyClientTest {
    static Server server;


    @BeforeClass
    public static void setUp() throws Exception {
        server = Exchangers.bind(URL.valueOf("exchange:    }

    @AfterClass
    public static void tearDown() throws Exception {
        try {
            if (server != null)
                server.close();
        } finally {
        }
    }

    public static void main(String[] args) throws RemotingException, InterruptedException {
        ExchangeChannel client = Exchangers.connect(URL.valueOf("exchange:        Thread.sleep(60 * 1000 * 50);
    }

    @Test
    public void testClientClose() throws Exception {
        List<ExchangeChannel> clients = new ArrayList<ExchangeChannel>(100);
        for (int i = 0; i < 100; i++) {
            ExchangeChannel client = Exchangers.connect(URL.valueOf("exchange:            Thread.sleep(5);
            clients.add(client);
        }
        for (ExchangeChannel client : clients) {
            client.close();
        }
        Thread.sleep(1000);
    }

    @Test
    public void testServerClose() throws Exception {
        for (int i = 0; i < 100; i++) {
            Server aServer = Exchangers.bind(URL.valueOf("exchange:            aServer.close();
        }
    }
}