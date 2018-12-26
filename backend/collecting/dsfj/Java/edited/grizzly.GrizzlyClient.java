
package com.alibaba.dubbo.remoting.transport.grizzly;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.transport.AbstractClient;

import org.glassfish.grizzly.Connection;
import org.glassfish.grizzly.filterchain.FilterChainBuilder;
import org.glassfish.grizzly.filterchain.TransportFilter;
import org.glassfish.grizzly.nio.transport.TCPNIOTransport;
import org.glassfish.grizzly.nio.transport.TCPNIOTransportBuilder;
import org.glassfish.grizzly.strategies.SameThreadIOStrategy;
import org.glassfish.grizzly.threadpool.ThreadPoolConfig;

import java.util.concurrent.TimeUnit;


public class GrizzlyClient extends AbstractClient {

    private static final Logger logger = LoggerFactory.getLogger(GrizzlyClient.class);

    private TCPNIOTransport transport;

    private volatile Connection<?> connection; 
    public GrizzlyClient(URL url, ChannelHandler handler) throws RemotingException {
        super(url, handler);
    }

    @Override
    protected void doOpen() throws Throwable {
        FilterChainBuilder filterChainBuilder = FilterChainBuilder.stateless();
        filterChainBuilder.add(new TransportFilter());
        filterChainBuilder.add(new GrizzlyCodecAdapter(getCodec(), getUrl(), this));
        filterChainBuilder.add(new GrizzlyHandler(getUrl(), this));
        TCPNIOTransportBuilder builder = TCPNIOTransportBuilder.newInstance();
        ThreadPoolConfig config = builder.getWorkerThreadPoolConfig();
        config.setPoolName(CLIENT_THREAD_POOL_NAME)
                .setQueueLimit(-1)
                .setCorePoolSize(0)
                .setMaxPoolSize(Integer.MAX_VALUE)
                .setKeepAliveTime(60L, TimeUnit.SECONDS);
        builder.setTcpNoDelay(true).setKeepAlive(true)
                .setConnectionTimeout(getTimeout())
                .setIOStrategy(SameThreadIOStrategy.getInstance());
        transport = builder.build();
        transport.setProcessor(filterChainBuilder.build());
        transport.start();
    }


    @Override
    protected void doConnect() throws Throwable {
        connection = transport.connect(getConnectAddress())
                .get(getUrl().getPositiveParameter(Constants.TIMEOUT_KEY, Constants.DEFAULT_TIMEOUT), TimeUnit.MILLISECONDS);
    }

    @Override
    protected void doDisConnect() throws Throwable {
        try {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        } catch (Throwable t) {
            logger.warn(t.getMessage());
        }
    }

    @Override
    protected void doClose() throws Throwable {
        try {
            transport.stop();
        } catch (Throwable e) {
            logger.warn(e.getMessage(), e);
        }
    }

    @Override
    protected Channel getChannel() {
        Connection<?> c = connection;
        if (c == null || !c.isOpen())
            return null;
        return GrizzlyChannel.getOrAddChannel(c, getUrl(), this);
    }

}