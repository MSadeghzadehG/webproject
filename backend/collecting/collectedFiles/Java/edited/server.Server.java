
package com.alibaba.dubbo.qos.server;

import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.qos.server.handler.QosProcessHandler;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.util.concurrent.DefaultThreadFactory;

import java.util.concurrent.atomic.AtomicBoolean;


public class Server {

    private static final Logger logger = LoggerFactory.getLogger(Server.class);
    private static final Server INSTANCE = new Server();

    public static final Server getInstance() {
        return INSTANCE;
    }

    private int port;

    private boolean acceptForeignIp = true;

    private EventLoopGroup boss;

    private EventLoopGroup worker;

    private Server() {
        this.welcome = DubboLogo.dubbo;
    }

    private String welcome;

    private AtomicBoolean hasStarted = new AtomicBoolean();

    
    public void setWelcome(String welcome) {
        this.welcome = welcome;
    }

    public int getPort() {
        return port;
    }

    
    public void start() throws Throwable {
        if (!hasStarted.compareAndSet(false, true)) {
            return;
        }
        boss = new NioEventLoopGroup(0, new DefaultThreadFactory("qos-boss", true));
        worker = new NioEventLoopGroup(0, new DefaultThreadFactory("qos-worker", true));
        ServerBootstrap serverBootstrap = new ServerBootstrap();
        serverBootstrap.group(boss, worker);
        serverBootstrap.channel(NioServerSocketChannel.class);
        serverBootstrap.childOption(ChannelOption.TCP_NODELAY, true);
        serverBootstrap.childOption(ChannelOption.SO_REUSEADDR, true);
        serverBootstrap.childHandler(new ChannelInitializer<Channel>() {

            @Override
            protected void initChannel(Channel ch) throws Exception {
                ch.pipeline().addLast(new QosProcessHandler(welcome, acceptForeignIp));
            }
        });
        try {
            serverBootstrap.bind(port).sync();
            logger.info("qos-server bind localhost:" + port);
        } catch (Throwable throwable) {
            logger.error("qos-server can not bind localhost:" + port, throwable);
            throw throwable;
        }
    }

    
    public void stop() {
        logger.info("qos-server stopped.");
        if (boss != null) {
            boss.shutdownGracefully();
        }
        if (worker != null) {
            worker.shutdownGracefully();
        }
    }

    public void setPort(int port) {
        this.port = port;
    }

    public boolean isAcceptForeignIp() {
        return acceptForeignIp;
    }

    public void setAcceptForeignIp(boolean acceptForeignIp) {
        this.acceptForeignIp = acceptForeignIp;
    }

    public String getWelcome() {
        return welcome;
    }
}
