

package org.elasticsearch.nio;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.stream.Collectors;


public class AcceptingSelector extends ESSelector {

    private final AcceptorEventHandler eventHandler;
    private final ConcurrentLinkedQueue<NioServerSocketChannel> newChannels = new ConcurrentLinkedQueue<>();

    public AcceptingSelector(AcceptorEventHandler eventHandler) throws IOException {
        super(eventHandler);
        this.eventHandler = eventHandler;
    }

    public AcceptingSelector(AcceptorEventHandler eventHandler, Selector selector) throws IOException {
        super(eventHandler, selector);
        this.eventHandler = eventHandler;
    }

    @Override
    void processKey(SelectionKey selectionKey) {
        ServerChannelContext channelContext = (ServerChannelContext) selectionKey.attachment();
        if (selectionKey.isAcceptable()) {
            try {
                eventHandler.acceptChannel(channelContext);
            } catch (IOException e) {
                eventHandler.acceptException(channelContext, e);
            }
        }
    }

    @Override
    void preSelect() {
        setUpNewServerChannels();
    }

    @Override
    void cleanup() {
        channelsToClose.addAll(newChannels.stream().map(NioServerSocketChannel::getContext).collect(Collectors.toList()));
    }

    
    public void scheduleForRegistration(NioServerSocketChannel serverSocketChannel) {
        newChannels.add(serverSocketChannel);
        ensureSelectorOpenForEnqueuing(newChannels, serverSocketChannel);
        wakeup();
    }

    private void setUpNewServerChannels() {
        NioServerSocketChannel newChannel;
        while ((newChannel = this.newChannels.poll()) != null) {
            ServerChannelContext context = newChannel.getContext();
            assert context.getSelector() == this : "The channel must be registered with the selector with which it was created";
            try {
                if (context.isOpen()) {
                    eventHandler.handleRegistration(context);
                } else {
                    eventHandler.registrationException(context, new ClosedChannelException());
                }
            } catch (Exception e) {
                eventHandler.registrationException(context, e);
            }
        }
    }
}
