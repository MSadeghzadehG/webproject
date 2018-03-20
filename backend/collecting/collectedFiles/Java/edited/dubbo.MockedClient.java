
package com.alibaba.dubbo.registry.dubbo;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.Codec;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.remoting.exchange.ExchangeHandler;
import com.alibaba.dubbo.remoting.exchange.ResponseCallback;
import com.alibaba.dubbo.remoting.exchange.ResponseFuture;
import com.alibaba.dubbo.remoting.exchange.support.Replier;

import java.net.InetSocketAddress;
import java.util.Map;


public class MockedClient implements ExchangeClient {

    
    
    private boolean connected;

    private Object received;

    private Object sent;

    private Object invoked;

    private Replier<?> handler;

    private InetSocketAddress address;

    private boolean closed = false;

    
    public MockedClient(String host, int port, boolean connected) {
        this(host, port, connected, null);
    }

    public MockedClient(String host, int port, boolean connected, Object received) {
        this.address = new InetSocketAddress(host, port);
        this.connected = connected;
        this.received = received;
    }

    public void open() {
    }

    public void close() {
        this.closed = true;
    }

    public void send(Object msg) throws RemotingException {
        this.sent = msg;
    }

    public ResponseFuture request(Object msg) throws RemotingException {
        return request(msg, 0);
    }

    public ResponseFuture request(Object msg, int timeout) throws RemotingException {
        this.invoked = msg;
        return new ResponseFuture() {
            public Object get() throws RemotingException {
                return received;
            }

            public Object get(int timeoutInMillis) throws RemotingException {
                return received;
            }

            public boolean isDone() {
                return true;
            }

            public void setCallback(ResponseCallback callback) {
            }
        };
    }

    public void registerHandler(Replier<?> handler) {
        this.handler = handler;
    }

    public void unregisterHandler(Replier<?> handler) {
            }

    public void addChannelListener(ChannelHandler listener) {
            }

    public void removeChannelListener(ChannelHandler listener) {
            }

    public boolean isConnected() {
        return connected;
    }

    
    public void setConnected(boolean connected) {
        this.connected = connected;
    }

    public Object getSent() {
        return sent;
    }

    public Replier<?> getHandler() {
        return handler;
    }

    public Object getInvoked() {
        return invoked;
    }

    public InetSocketAddress getRemoteAddress() {
        return address;
    }

    public String getName() {
        return "mocked";
    }

    public InetSocketAddress getLocalAddress() {
        return null;
    }

    public int getTimeout() {
        return 0;
    }

    public void setTimeout(int timeout) {
    }

    public void close(int timeout) {
        close();
    }

    @Override
    public void startClose() {

    }

    public boolean isOpen() {
        return closed;
    }

    public Codec getCodec() {
        return null;
    }

    public void setCodec(Codec codec) {
    }

    public String getHost() {
        return null;
    }

    public void setHost(String host) {
    }

    public int getPort() {
        return 0;
    }

    public void setPort(int port) {
    }

    public int getThreadCount() {
        return 0;
    }

    public void setThreadCount(int threadCount) {
    }

    public URL getUrl() {
        return null;
    }

    public Replier<?> getReceiver() {
        return null;
    }

    public ChannelHandler getChannelHandler() {
        return null;
    }

    public void reset(Map<String, String> parameters) {
    }

    public Channel getChannel() {
        return this;
    }

    public ExchangeHandler getExchangeHandler() {
        return null;
    }

    public void reconnect() throws RemotingException {
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
        return closed;
    }

    public void removeAttribute(String key) {

    }

    
    public Object getReceived() {
        return received;
    }

    
    public void setReceived(Object received) {
        this.received = received;
    }

    public void send(Object message, boolean sent) throws RemotingException {
    }

    public void reset(URL url) {
    }

    @Deprecated
    public void reset(com.alibaba.dubbo.common.Parameters parameters) {
    }

}