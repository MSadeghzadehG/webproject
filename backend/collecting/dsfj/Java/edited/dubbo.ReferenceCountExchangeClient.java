
package com.alibaba.dubbo.rpc.protocol.dubbo;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.Parameters;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.remoting.exchange.ExchangeHandler;
import com.alibaba.dubbo.remoting.exchange.ResponseFuture;

import java.net.InetSocketAddress;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicInteger;


@SuppressWarnings("deprecation")
final class ReferenceCountExchangeClient implements ExchangeClient {

    private final URL url;
    private final AtomicInteger refenceCount = new AtomicInteger(0);

        private final ConcurrentMap<String, LazyConnectExchangeClient> ghostClientMap;
    private ExchangeClient client;


    public ReferenceCountExchangeClient(ExchangeClient client, ConcurrentMap<String, LazyConnectExchangeClient> ghostClientMap) {
        this.client = client;
        refenceCount.incrementAndGet();
        this.url = client.getUrl();
        if (ghostClientMap == null) {
            throw new IllegalStateException("ghostClientMap can not be null, url: " + url);
        }
        this.ghostClientMap = ghostClientMap;
    }

    public void reset(URL url) {
        client.reset(url);
    }

    public ResponseFuture request(Object request) throws RemotingException {
        return client.request(request);
    }

    public URL getUrl() {
        return client.getUrl();
    }

    public InetSocketAddress getRemoteAddress() {
        return client.getRemoteAddress();
    }

    public ChannelHandler getChannelHandler() {
        return client.getChannelHandler();
    }

    public ResponseFuture request(Object request, int timeout) throws RemotingException {
        return client.request(request, timeout);
    }

    public boolean isConnected() {
        return client.isConnected();
    }

    public void reconnect() throws RemotingException {
        client.reconnect();
    }

    public InetSocketAddress getLocalAddress() {
        return client.getLocalAddress();
    }

    public boolean hasAttribute(String key) {
        return client.hasAttribute(key);
    }

    public void reset(Parameters parameters) {
        client.reset(parameters);
    }

    public void send(Object message) throws RemotingException {
        client.send(message);
    }

    public ExchangeHandler getExchangeHandler() {
        return client.getExchangeHandler();
    }

    public Object getAttribute(String key) {
        return client.getAttribute(key);
    }

    public void send(Object message, boolean sent) throws RemotingException {
        client.send(message, sent);
    }

    public void setAttribute(String key, Object value) {
        client.setAttribute(key, value);
    }

    public void removeAttribute(String key) {
        client.removeAttribute(key);
    }

    
    public void close() {
        close(0);
    }

    public void close(int timeout) {
        if (refenceCount.decrementAndGet() <= 0) {
            if (timeout == 0) {
                client.close();
            } else {
                client.close(timeout);
            }
            client = replaceWithLazyClient();
        }
    }

    public void startClose() {
        client.startClose();
    }

        private LazyConnectExchangeClient replaceWithLazyClient() {
                URL lazyUrl = url.addParameter(Constants.LAZY_CONNECT_INITIAL_STATE_KEY, Boolean.FALSE)
                .addParameter(Constants.RECONNECT_KEY, Boolean.FALSE)
                .addParameter(Constants.SEND_RECONNECT_KEY, Boolean.TRUE.toString())
                .addParameter("warning", Boolean.TRUE.toString())
                .addParameter(LazyConnectExchangeClient.REQUEST_WITH_WARNING_KEY, true)
                .addParameter("_client_memo", "referencecounthandler.replacewithlazyclient");

        String key = url.getAddress();
                LazyConnectExchangeClient gclient = ghostClientMap.get(key);
        if (gclient == null || gclient.isClosed()) {
            gclient = new LazyConnectExchangeClient(lazyUrl, client.getExchangeHandler());
            ghostClientMap.put(key, gclient);
        }
        return gclient;
    }

    public boolean isClosed() {
        return client.isClosed();
    }

    public void incrementAndGetCount() {
        refenceCount.incrementAndGet();
    }
}