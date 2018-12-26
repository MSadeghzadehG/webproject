
package com.alibaba.dubbo.rpc.protocol.rest;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;

import io.netty.channel.ChannelOption;
import org.jboss.resteasy.plugins.server.netty.NettyJaxrsServer;
import org.jboss.resteasy.spi.ResteasyDeployment;

import java.util.HashMap;
import java.util.Map;


public class NettyServer extends BaseRestServer {

    private final NettyJaxrsServer server = new NettyJaxrsServer();

    protected void doStart(URL url) {
        String bindIp = url.getParameter(Constants.BIND_IP_KEY, url.getHost());
        if (!url.isAnyHost() && NetUtils.isValidLocalHost(bindIp)) {
            server.setHostname(bindIp);
        }
        server.setPort(url.getParameter(Constants.BIND_PORT_KEY, url.getPort()));
        Map<ChannelOption, Object> channelOption = new HashMap<ChannelOption, Object>();
        channelOption.put(ChannelOption.SO_KEEPALIVE, url.getParameter(Constants.KEEP_ALIVE_KEY, Constants.DEFAULT_KEEP_ALIVE));
        server.setChildChannelOptions(channelOption);
        server.setExecutorThreadCount(url.getParameter(Constants.THREADS_KEY, Constants.DEFAULT_THREADS));
        server.setIoWorkerCount(url.getParameter(Constants.IO_THREADS_KEY, Constants.DEFAULT_IO_THREADS));
        server.setMaxRequestSize(url.getParameter(Constants.PAYLOAD_KEY, Constants.DEFAULT_PAYLOAD));
        server.start();
    }

    public void stop() {
        server.stop();
    }

    protected ResteasyDeployment getDeployment() {
        return server.getDeployment();
    }
}
