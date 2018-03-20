

package org.elasticsearch.nio;

import java.io.IOException;
import java.nio.channels.ServerSocketChannel;
import java.util.concurrent.atomic.AtomicBoolean;

public class NioServerSocketChannel extends NioChannel {

    private final ServerSocketChannel socketChannel;
    private final AtomicBoolean contextSet = new AtomicBoolean(false);
    private ServerChannelContext context;

    public NioServerSocketChannel(ServerSocketChannel socketChannel) throws IOException {
        super(socketChannel);
        this.socketChannel = socketChannel;
    }

    
    public void setContext(ServerChannelContext context) {
        if (contextSet.compareAndSet(false, true)) {
            this.context = context;
        } else {
            throw new IllegalStateException("Context on this channel were already set. It should only be once.");
        }
    }

    @Override
    public ServerSocketChannel getRawChannel() {
        return socketChannel;
    }

    @Override
    public ServerChannelContext getContext() {
        return context;
    }

    @Override
    public String toString() {
        return "NioServerSocketChannel{" +
            "localAddress=" + getLocalAddress() +
            '}';
    }
}
