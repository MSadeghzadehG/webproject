
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.remoting.exchange.Exchangers;
import com.alibaba.dubbo.remoting.exchange.support.ExchangeHandlerAdapter;

import junit.framework.TestCase;
import org.junit.Test;


public class ChanelHandlerTest extends TestCase {

    private static final Logger logger = LoggerFactory.getLogger(ChanelHandlerTest.class);

    public static ExchangeClient initClient(String url) {
                ExchangeClient exchangeClient = null;
        PeformanceTestHandler handler = new PeformanceTestHandler(url);
        boolean run = true;
        while (run) {
            try {
                exchangeClient = Exchangers.connect(url, handler);
            } catch (Throwable t) {

                if (t != null && t.getCause() != null && t.getCause().getClass() != null && (t.getCause().getClass() == java.net.ConnectException.class
                        || t.getCause().getClass() == java.net.ConnectException.class)) {

                } else {
                    t.printStackTrace();
                }

                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (exchangeClient != null) {
                run = false;
            }
        }
        return exchangeClient;
    }

    public static void closeClient(ExchangeClient client) {
        if (client.isConnected()) {
            client.close();
        }
    }

    @Test
    public void testClient() throws Throwable {
                if (PerformanceUtils.getProperty("server", null) == null) {
            logger.warn("Please set -Dserver=127.0.0.1:9911");
            return;
        }
        final String server = System.getProperty("server", "127.0.0.1:9911");
        final String transporter = PerformanceUtils.getProperty(Constants.TRANSPORTER_KEY, Constants.DEFAULT_TRANSPORTER);
        final String serialization = PerformanceUtils.getProperty(Constants.SERIALIZATION_KEY, Constants.DEFAULT_REMOTING_SERIALIZATION);
        final int timeout = PerformanceUtils.getIntProperty(Constants.TIMEOUT_KEY, Constants.DEFAULT_TIMEOUT);
        int sleep = PerformanceUtils.getIntProperty("sleep", 60 * 1000 * 60);

        final String url = "exchange:        ExchangeClient exchangeClient = initClient(url);
        Thread.sleep(sleep);
        closeClient(exchangeClient);
    }

    static class PeformanceTestHandler extends ExchangeHandlerAdapter {
        String url = "";

        
        public PeformanceTestHandler(String url) {
            this.url = url;
        }

        public void connected(Channel channel) throws RemotingException {
            System.out.println("connected event,chanel;" + channel);
        }

        public void disconnected(Channel channel) throws RemotingException {
            System.out.println("disconnected event,chanel;" + channel);
            initClient(url);
        }

        
        @Override
        public void caught(Channel channel, Throwable exception) throws RemotingException {
        }


    }
}