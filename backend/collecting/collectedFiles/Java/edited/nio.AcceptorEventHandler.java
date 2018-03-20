

package org.elasticsearch.nio;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.util.function.Supplier;


public class AcceptorEventHandler extends EventHandler {

    private final Supplier<SocketSelector> selectorSupplier;

    public AcceptorEventHandler(Logger logger, Supplier<SocketSelector> selectorSupplier) {
        super(logger);
        this.selectorSupplier = selectorSupplier;
    }

    
    protected void handleRegistration(ServerChannelContext context) throws IOException {
        context.register();
        SelectionKey selectionKey = context.getSelectionKey();
        selectionKey.attach(context);
        SelectionKeyUtils.setAcceptInterested(selectionKey);
    }

    
    protected void registrationException(ServerChannelContext context, Exception exception) {
        logger.error(new ParameterizedMessage("failed to register server channel: {}", context.getChannel()), exception);
    }

    
    protected void acceptChannel(ServerChannelContext context) throws IOException {
        context.acceptChannels(selectorSupplier);
    }

    
    protected void acceptException(ServerChannelContext context, Exception exception) {
        logger.debug(() -> new ParameterizedMessage("exception while accepting new channel from server channel: {}",
            context.getChannel()), exception);
    }
}
