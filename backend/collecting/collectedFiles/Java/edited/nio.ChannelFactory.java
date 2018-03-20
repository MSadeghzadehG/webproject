

package org.elasticsearch.nio;

import java.io.Closeable;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.function.Supplier;

public abstract class ChannelFactory<ServerSocket extends NioServerSocketChannel, Socket extends NioSocketChannel> {

    private final ChannelFactory.RawChannelFactory rawChannelFactory;

    
    protected ChannelFactory(RawChannelFactory rawChannelFactory) {
        this.rawChannelFactory = rawChannelFactory;
    }

    public Socket openNioChannel(InetSocketAddress remoteAddress, Supplier<SocketSelector> supplier) throws IOException {
        SocketChannel rawChannel = rawChannelFactory.openNioChannel(remoteAddress);
        SocketSelector selector = supplier.get();
        Socket channel = internalCreateChannel(selector, rawChannel);
        scheduleChannel(channel, selector);
        return channel;
    }

    public Socket acceptNioChannel(ServerChannelContext serverContext, Supplier<SocketSelector> supplier) throws IOException {
        SocketChannel rawChannel = rawChannelFactory.acceptNioChannel(serverContext);
                if (rawChannel == null) {
            return null;
        } else {
            SocketSelector selector = supplier.get();
            Socket channel = internalCreateChannel(selector, rawChannel);
            scheduleChannel(channel, selector);
            return channel;
        }
    }

    public ServerSocket openNioServerSocketChannel(InetSocketAddress address, Supplier<AcceptingSelector> supplier) throws IOException {
        ServerSocketChannel rawChannel = rawChannelFactory.openNioServerSocketChannel(address);
        AcceptingSelector selector = supplier.get();
        ServerSocket serverChannel = internalCreateServerChannel(selector, rawChannel);
        scheduleServerChannel(serverChannel, selector);
        return serverChannel;
    }

    
    public abstract Socket createChannel(SocketSelector selector, SocketChannel channel) throws IOException;

    
    public abstract ServerSocket createServerChannel(AcceptingSelector selector, ServerSocketChannel channel) throws IOException;

    private Socket internalCreateChannel(SocketSelector selector, SocketChannel rawChannel) throws IOException {
        try {
            Socket channel = createChannel(selector, rawChannel);
            assert channel.getContext() != null : "channel context should have been set on channel";
            return channel;
        } catch (Exception e) {
            closeRawChannel(rawChannel, e);
            throw e;
        }
    }

    private ServerSocket internalCreateServerChannel(AcceptingSelector selector, ServerSocketChannel rawChannel) throws IOException {
        try {
            return createServerChannel(selector, rawChannel);
        } catch (Exception e) {
            closeRawChannel(rawChannel, e);
            throw e;
        }
    }

    private void scheduleChannel(Socket channel, SocketSelector selector) {
        try {
            selector.scheduleForRegistration(channel);
        } catch (IllegalStateException e) {
            closeRawChannel(channel.getRawChannel(), e);
            throw e;
        }
    }

    private void scheduleServerChannel(ServerSocket channel, AcceptingSelector selector) {
        try {
            selector.scheduleForRegistration(channel);
        } catch (IllegalStateException e) {
            closeRawChannel(channel.getRawChannel(), e);
            throw e;
        }
    }

    private static void closeRawChannel(Closeable c, Exception e) {
        try {
            c.close();
        } catch (IOException closeException) {
            e.addSuppressed(closeException);
        }
    }

    protected static class RawChannelFactory {

        private final boolean tcpNoDelay;
        private final boolean tcpKeepAlive;
        private final boolean tcpReusedAddress;
        private final int tcpSendBufferSize;
        private final int tcpReceiveBufferSize;

        public RawChannelFactory(boolean tcpNoDelay, boolean tcpKeepAlive, boolean tcpReusedAddress, int tcpSendBufferSize,
                                 int tcpReceiveBufferSize) {
            this.tcpNoDelay = tcpNoDelay;
            this.tcpKeepAlive = tcpKeepAlive;
            this.tcpReusedAddress = tcpReusedAddress;
            this.tcpSendBufferSize = tcpSendBufferSize;
            this.tcpReceiveBufferSize = tcpReceiveBufferSize;
        }

        SocketChannel openNioChannel(InetSocketAddress remoteAddress) throws IOException {
            SocketChannel socketChannel = SocketChannel.open();
            try {
                configureSocketChannel(socketChannel);
                connect(socketChannel, remoteAddress);
            } catch (IOException e) {
                closeRawChannel(socketChannel, e);
                throw e;
            }
            return socketChannel;
        }

        SocketChannel acceptNioChannel(ServerChannelContext serverContext) throws IOException {
            ServerSocketChannel rawChannel = serverContext.getChannel().getRawChannel();
            SocketChannel socketChannel = accept(rawChannel);
            if (socketChannel == null) {
                return null;
            }
            try {
                configureSocketChannel(socketChannel);
            } catch (IOException e) {
                closeRawChannel(socketChannel, e);
                throw e;
            }
            return socketChannel;
        }

        ServerSocketChannel openNioServerSocketChannel(InetSocketAddress address) throws IOException {
            ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();
            serverSocketChannel.configureBlocking(false);
            java.net.ServerSocket socket = serverSocketChannel.socket();
            try {
                socket.setReuseAddress(tcpReusedAddress);
                serverSocketChannel.bind(address);
            } catch (IOException e) {
                closeRawChannel(serverSocketChannel, e);
                throw e;
            }
            return serverSocketChannel;
        }

        private void configureSocketChannel(SocketChannel channel) throws IOException {
            channel.configureBlocking(false);
            java.net.Socket socket = channel.socket();
            socket.setTcpNoDelay(tcpNoDelay);
            socket.setKeepAlive(tcpKeepAlive);
            socket.setReuseAddress(tcpReusedAddress);
            if (tcpSendBufferSize > 0) {
                socket.setSendBufferSize(tcpSendBufferSize);
            }
            if (tcpReceiveBufferSize > 0) {
                socket.setSendBufferSize(tcpReceiveBufferSize);
            }
        }

        public static SocketChannel accept(ServerSocketChannel serverSocketChannel) throws IOException {
            try {
                return AccessController.doPrivileged((PrivilegedExceptionAction<SocketChannel>) serverSocketChannel::accept);
            } catch (PrivilegedActionException e) {
                throw (IOException) e.getCause();
            }
        }

        private static void connect(SocketChannel socketChannel, InetSocketAddress remoteAddress) throws IOException {
            try {
                AccessController.doPrivileged((PrivilegedExceptionAction<Boolean>) () -> socketChannel.connect(remoteAddress));
            } catch (PrivilegedActionException e) {
                throw (IOException) e.getCause();
            }
        }
    }
}
