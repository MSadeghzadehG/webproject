
package com.alibaba.dubbo.remoting.handler;

import com.alibaba.dubbo.common.utils.ConcurrentHashSet;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.RemotingException;

import java.util.Collections;
import java.util.Set;

public class MockedChannelHandler implements ChannelHandler {
        ConcurrentHashSet<Channel> channels = new ConcurrentHashSet<Channel>();

    public void connected(Channel channel) throws RemotingException {
        channels.add(channel);
    }

    public void disconnected(Channel channel) throws RemotingException {
        channels.remove(channel);
    }

    public void sent(Channel channel, Object message) throws RemotingException {
        channel.send(message);
    }

    public void received(Channel channel, Object message) throws RemotingException {
                channel.send(message);
    }

    public void caught(Channel channel, Throwable exception) throws RemotingException {
        throw new RemotingException(channel, exception);

    }

    public Set<Channel> getChannels() {
        return Collections.unmodifiableSet(channels);
    }
}
