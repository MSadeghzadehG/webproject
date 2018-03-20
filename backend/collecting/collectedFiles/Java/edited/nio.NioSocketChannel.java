

package org.elasticsearch.nio;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.SocketChannel;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.BiConsumer;

public class NioSocketChannel extends NioChannel {

    private final InetSocketAddress remoteAddress;
    private final AtomicBoolean contextSet = new AtomicBoolean(false);
    private final SocketChannel socketChannel;
    private SocketChannelContext context;

    public NioSocketChannel(SocketChannel socketChannel) throws IOException {
        super(socketChannel);
        this.socketChannel = socketChannel;
        this.remoteAddress = (InetSocketAddress) socketChannel.getRemoteAddress();
    }

    public void setContext(SocketChannelContext context) {
        if (contextSet.compareAndSet(false, true)) {
            this.context = context;
        } else {
            throw new IllegalStateException("Context on this channel were already set. It should only be once.");
        }
    }

    @Override
    public SocketChannel getRawChannel() {
        return socketChannel;
    }

    @Override
    public SocketChannelContext getContext() {
        return context;
    }

    public InetSocketAddress getRemoteAddress() {
        return remoteAddress;
    }

    public void addConnectListener(BiConsumer<Void, Throwable> listener) {
        context.addConnectListener(listener);
    }

    @Override
    public String toString() {
        return "NioSocketChannel{" +
            "localAddress=" + getLocalAddress() +
            ", remoteAddress=" + remoteAddress +
            '}';
    }
}
