

package com.alibaba.dubbo.remoting.exchange.support.header;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MockChannel implements Channel {

    private Map<String, Object> attributes = new HashMap<String, Object>();

    private volatile boolean closed = false;
    private volatile boolean closing = false;
    private List<Object> sentObjects = new ArrayList<Object>();

    public InetSocketAddress getRemoteAddress() {
        return null;
    }

    public boolean isConnected() {
        return false;
    }

    public boolean hasAttribute(String key) {
        return attributes.containsKey(key);
    }

    public Object getAttribute(String key) {
        return attributes.get(key);
    }

    public void setAttribute(String key, Object value) {
        attributes.put(key, value);
    }

    public void removeAttribute(String key) {
        attributes.remove(key);
    }

    public URL getUrl() {
        return null;
    }

    public ChannelHandler getChannelHandler() {
        return null;
    }

    public InetSocketAddress getLocalAddress() {
        return null;
    }

    public void send(Object message) throws RemotingException {
        sentObjects.add(message);
    }

    public void send(Object message, boolean sent) throws RemotingException {
        sentObjects.add(message);
    }

    public void close() {
        closed = true;
    }

    public void close(int timeout) {
        closed = true;
    }

    @Override
    public void startClose() {
        closing = true;
    }

    public boolean isClosed() {
        return closed;
    }

    public List<Object> getSentObjects() {
        return Collections.unmodifiableList(sentObjects);
    }
}
