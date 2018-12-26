

package org.elasticsearch.transport.nio;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.nio.AcceptingSelector;
import org.elasticsearch.nio.ChannelFactory;
import org.elasticsearch.nio.NioServerSocketChannel;
import org.elasticsearch.transport.TcpChannel;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.ServerSocketChannel;


public class TcpNioServerSocketChannel extends NioServerSocketChannel implements TcpChannel {

    private final String profile;

    public TcpNioServerSocketChannel(String profile, ServerSocketChannel socketChannel) throws IOException {
        super(socketChannel);
        this.profile = profile;
    }

    @Override
    public void sendMessage(BytesReference reference, ActionListener<Void> listener) {
        throw new UnsupportedOperationException("Cannot send a message to a server channel.");
    }

    @Override
    public void setSoLinger(int value) throws IOException {
        throw new UnsupportedOperationException("Cannot set SO_LINGER on a server channel.");
    }

    @Override
    public InetSocketAddress getRemoteAddress() {
        return null;
    }

    @Override
    public void close() {
        getContext().closeChannel();
    }

    @Override
    public String getProfile() {
        return profile;
    }

    @Override
    public void addCloseListener(ActionListener<Void> listener) {
        addCloseListener(ActionListener.toBiConsumer(listener));
    }

    @Override
    public String toString() {
        return "TcpNioServerSocketChannel{" +
            "localAddress=" + getLocalAddress() +
            '}';
    }
}
