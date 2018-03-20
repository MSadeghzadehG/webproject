
package com.alibaba.dubbo.remoting.transport.grizzly;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.StringUtils;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;

import org.glassfish.grizzly.Connection;
import org.glassfish.grizzly.filterchain.BaseFilter;
import org.glassfish.grizzly.filterchain.FilterChainContext;
import org.glassfish.grizzly.filterchain.NextAction;

import java.io.IOException;


public class GrizzlyHandler extends BaseFilter {

    private static final Logger logger = LoggerFactory.getLogger(GrizzlyHandler.class);

    private final URL url;

    private final ChannelHandler handler;

    public GrizzlyHandler(URL url, ChannelHandler handler) {
        this.url = url;
        this.handler = handler;
    }

    @Override
    public NextAction handleConnect(FilterChainContext ctx) throws IOException {
        Connection<?> connection = ctx.getConnection();
        GrizzlyChannel channel = GrizzlyChannel.getOrAddChannel(connection, url, handler);
        try {
            handler.connected(channel);
        } catch (RemotingException e) {
            throw new IOException(StringUtils.toString(e));
        } finally {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        }
        return ctx.getInvokeAction();
    }

    @Override
    public NextAction handleClose(FilterChainContext ctx) throws IOException {
        Connection<?> connection = ctx.getConnection();
        GrizzlyChannel channel = GrizzlyChannel.getOrAddChannel(connection, url, handler);
        try {
            handler.disconnected(channel);
        } catch (RemotingException e) {
            throw new IOException(StringUtils.toString(e));
        } finally {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        }
        return ctx.getInvokeAction();
    }

    @Override
    public NextAction handleRead(FilterChainContext ctx) throws IOException {
        Connection<?> connection = ctx.getConnection();
        GrizzlyChannel channel = GrizzlyChannel.getOrAddChannel(connection, url, handler);
        try {
            handler.received(channel, ctx.getMessage());
        } catch (RemotingException e) {
            throw new IOException(StringUtils.toString(e));
        } finally {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        }
        return ctx.getInvokeAction();
    }

    @Override
    public NextAction handleWrite(FilterChainContext ctx) throws IOException {
        Connection<?> connection = ctx.getConnection();
        GrizzlyChannel channel = GrizzlyChannel.getOrAddChannel(connection, url, handler);
        try {
            handler.sent(channel, ctx.getMessage());
        } catch (RemotingException e) {
            throw new IOException(StringUtils.toString(e));
        } finally {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        }
        return ctx.getInvokeAction();
    }

    @Override
    public void exceptionOccurred(FilterChainContext ctx, Throwable error) {
        Connection<?> connection = ctx.getConnection();
        GrizzlyChannel channel = GrizzlyChannel.getOrAddChannel(connection, url, handler);
        try {
            handler.caught(channel, error);
        } catch (RemotingException e) {
            logger.error("RemotingException on channel " + channel, e);
        } finally {
            GrizzlyChannel.removeChannelIfDisconnectd(connection);
        }
    }

}