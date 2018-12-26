

package org.elasticsearch.nio;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.NetworkChannel;
import java.util.function.BiConsumer;


public abstract class NioChannel {

    private final InetSocketAddress localAddress;

    NioChannel(NetworkChannel socketChannel) throws IOException {
        this.localAddress = (InetSocketAddress) socketChannel.getLocalAddress();
    }

    public boolean isOpen() {
        return getContext().isOpen();
    }

    public InetSocketAddress getLocalAddress() {
        return localAddress;
    }

    
    public void addCloseListener(BiConsumer<Void, Throwable> listener) {
        getContext().addCloseListener(listener);
    }

    
    public void close() {
        getContext().closeChannel();
    }

    public abstract NetworkChannel getRawChannel();

    public abstract ChannelContext<?> getContext();
}
