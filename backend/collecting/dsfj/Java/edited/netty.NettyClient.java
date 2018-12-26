
package com.alibaba.dubbo.remoting.transport.netty;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.Version;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.NamedThreadFactory;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.transport.AbstractClient;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;


public class NettyClient extends AbstractClient {

    private static final Logger logger = LoggerFactory.getLogger(NettyClient.class);

            private static final ChannelFactory channelFactory = new NioClientSocketChannelFactory(Executors.newCachedThreadPool(new NamedThreadFactory("NettyClientBoss", true)),
            Executors.newCachedThreadPool(new NamedThreadFactory("NettyClientWorker", true)),
            Constants.DEFAULT_IO_THREADS);
    private ClientBootstrap bootstrap;

    private volatile Channel channel; 
    public NettyClient(final URL url, final ChannelHandler handler) throws RemotingException {
        super(url, wrapChannelHandler(url, handler));
    }

    @Override
    protected void doOpen() throws Throwable {
        NettyHelper.setNettyLoggerFactory();
        bootstrap = new ClientBootstrap(channelFactory);
                        bootstrap.setOption("keepAlive", true);
        bootstrap.setOption("tcpNoDelay", true);
        bootstrap.setOption("connectTimeoutMillis", getTimeout());
        final NettyHandler nettyHandler = new NettyHandler(getUrl(), this);
        bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
            public ChannelPipeline getPipeline() {
                NettyCodecAdapter adapter = new NettyCodecAdapter(getCodec(), getUrl(), NettyClient.this);
                ChannelPipeline pipeline = Channels.pipeline();
                pipeline.addLast("decoder", adapter.getDecoder());
                pipeline.addLast("encoder", adapter.getEncoder());
                pipeline.addLast("handler", nettyHandler);
                return pipeline;
            }
        });
    }

    protected void doConnect() throws Throwable {
        long start = System.currentTimeMillis();
        ChannelFuture future = bootstrap.connect(getConnectAddress());
        try {
            boolean ret = future.awaitUninterruptibly(getConnectTimeout(), TimeUnit.MILLISECONDS);

            if (ret && future.isSuccess()) {
                Channel newChannel = future.getChannel();
                newChannel.setInterestOps(Channel.OP_READ_WRITE);
                try {
                                        Channel oldChannel = NettyClient.this.channel;                     if (oldChannel != null) {
                        try {
                            if (logger.isInfoEnabled()) {
                                logger.info("Close old netty channel " + oldChannel + " on create new netty channel " + newChannel);
                            }
                            oldChannel.close();
                        } finally {
                            NettyChannel.removeChannelIfDisconnected(oldChannel);
                        }
                    }
                } finally {
                    if (NettyClient.this.isClosed()) {
                        try {
                            if (logger.isInfoEnabled()) {
                                logger.info("Close new netty channel " + newChannel + ", because the client closed.");
                            }
                            newChannel.close();
                        } finally {
                            NettyClient.this.channel = null;
                            NettyChannel.removeChannelIfDisconnected(newChannel);
                        }
                    } else {
                        NettyClient.this.channel = newChannel;
                    }
                }
            } else if (future.getCause() != null) {
                throw new RemotingException(this, "client(url: " + getUrl() + ") failed to connect to server "
                        + getRemoteAddress() + ", error message is:" + future.getCause().getMessage(), future.getCause());
            } else {
                throw new RemotingException(this, "client(url: " + getUrl() + ") failed to connect to server "
                        + getRemoteAddress() + " client-side timeout "
                        + getConnectTimeout() + "ms (elapsed: " + (System.currentTimeMillis() - start) + "ms) from netty client "
                        + NetUtils.getLocalHost() + " using dubbo version " + Version.getVersion());
            }
        } finally {
            if (!isConnected()) {
                future.cancel();
            }
        }
    }

    @Override
    protected void doDisConnect() throws Throwable {
        try {
            NettyChannel.removeChannelIfDisconnected(channel);
        } catch (Throwable t) {
            logger.warn(t.getMessage());
        }
    }

    @Override
    protected void doClose() throws Throwable {
        
    }

    @Override
    protected com.alibaba.dubbo.remoting.Channel getChannel() {
        Channel c = channel;
        if (c == null || !c.isConnected())
            return null;
        return NettyChannel.getOrAddChannel(c, getUrl(), this);
    }

}