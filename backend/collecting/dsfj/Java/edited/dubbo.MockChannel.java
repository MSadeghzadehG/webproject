
package com.alibaba.dubbo.registry.dubbo;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.ExchangeChannel;
import com.alibaba.dubbo.remoting.exchange.ExchangeHandler;
import com.alibaba.dubbo.remoting.exchange.ResponseFuture;

import java.net.InetSocketAddress;

public class MockChannel implements ExchangeChannel {

    public static boolean closed = false;
    public static boolean closing = true;
    final InetSocketAddress localAddress;
    final InetSocketAddress remoteAddress;

    public MockChannel(String localHostname, int localPort, String remoteHostName, int remotePort) {
        localAddress = new InetSocketAddress(localHostname, localPort);
        remoteAddress = new InetSocketAddress(remoteHostName, remotePort);
        closed = false;
    }

    public InetSocketAddress getLocalAddress() {
        return localAddress;
    }

    public InetSocketAddress getRemoteAddress() {
        return remoteAddress;
    }

    public boolean isConnected() {
        return true;
    }

    public void close() {
        closed = true;
    }

    public void send(Object message) throws RemotingException {
    }

    public void close(int timeout) {
    }

    @Override
    public void startClose() {
        closing = true;
    }

    public URL getUrl() {
        return null;
    }

    public ResponseFuture send(Object request, int timeout) throws RemotingException {
        return null;
    }

    public ChannelHandler getChannelHandler() {
        return null;
    }

    public ResponseFuture request(Object request) throws RemotingException {
        return null;
    }

    public ResponseFuture request(Object request, int timeout) throws RemotingException {
        return null;
    }

    public ExchangeHandler getExchangeHandler() {
        return null;
    }

    public Object getAttribute(String key) {
        return null;
    }

    public void setAttribute(String key, Object value) {

    }

    public boolean hasAttribute(String key) {
        return false;
    }

    public boolean isClosed() {
        return false;
    }

    public void removeAttribute(String key) {

    }

    public void send(Object message, boolean sent) throws RemotingException {

    }

}