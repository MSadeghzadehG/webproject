

package org.elasticsearch.nio;

import java.io.IOException;
import java.nio.channels.ServerSocketChannel;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;
import java.util.function.Supplier;

public class ServerChannelContext extends ChannelContext<ServerSocketChannel> {

    private final NioServerSocketChannel channel;
    private final AcceptingSelector selector;
    private final Consumer<NioSocketChannel> acceptor;
    private final AtomicBoolean isClosing = new AtomicBoolean(false);
    private final ChannelFactory<?, ?> channelFactory;

    public ServerChannelContext(NioServerSocketChannel channel, ChannelFactory<?, ?> channelFactory, AcceptingSelector selector,
                                Consumer<NioSocketChannel> acceptor, Consumer<Exception> exceptionHandler) {
        super(channel.getRawChannel(), exceptionHandler);
        this.channel = channel;
        this.channelFactory = channelFactory;
        this.selector = selector;
        this.acceptor = acceptor;
    }

    public void acceptChannels(Supplier<SocketSelector> selectorSupplier) throws IOException {
        NioSocketChannel acceptedChannel;
        while ((acceptedChannel = channelFactory.acceptNioChannel(this, selectorSupplier)) != null) {
            acceptor.accept(acceptedChannel);
        }
    }

    @Override
    public void closeChannel() {
        if (isClosing.compareAndSet(false, true)) {
            getSelector().queueChannelClose(channel);
        }
    }

    @Override
    public AcceptingSelector getSelector() {
        return selector;
    }

    @Override
    public NioServerSocketChannel getChannel() {
        return channel;
    }

}
