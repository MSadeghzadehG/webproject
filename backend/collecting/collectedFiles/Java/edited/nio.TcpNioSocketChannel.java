

package org.elasticsearch.transport.nio;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.nio.NioSocketChannel;
import org.elasticsearch.nio.SocketSelector;
import org.elasticsearch.transport.TcpChannel;

import java.io.IOException;
import java.net.StandardSocketOptions;
import java.nio.channels.SocketChannel;

public class TcpNioSocketChannel extends NioSocketChannel implements TcpChannel {

    private final String profile;

    public TcpNioSocketChannel(String profile, SocketChannel socketChannel) throws IOException {
        super(socketChannel);
        this.profile = profile;
    }

    public void sendMessage(BytesReference reference, ActionListener<Void> listener) {
        getContext().sendMessage(BytesReference.toByteBuffers(reference), ActionListener.toBiConsumer(listener));
    }

    @Override
    public void setSoLinger(int value) throws IOException {
        if (isOpen()) {
            getRawChannel().setOption(StandardSocketOptions.SO_LINGER, value);
        }
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
    public void close() {
        getContext().closeChannel();
    }

    @Override
    public String toString() {
        return "TcpNioSocketChannel{" +
            "localAddress=" + getLocalAddress() +
            ", remoteAddress=" + getRemoteAddress() +
            '}';
    }
}
