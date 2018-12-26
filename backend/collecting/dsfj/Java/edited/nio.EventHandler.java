

package org.elasticsearch.nio;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;

import java.io.IOException;
import java.nio.channels.Selector;

public abstract class EventHandler {

    protected final Logger logger;

    EventHandler(Logger logger) {
        this.logger = logger;
    }

    
    protected void selectException(IOException exception) {
        logger.warn(new ParameterizedMessage("io exception during select [thread={}]", Thread.currentThread().getName()), exception);
    }

    
    protected void closeSelectorException(IOException exception) {
        logger.warn(new ParameterizedMessage("io exception while closing selector [thread={}]", Thread.currentThread().getName()),
            exception);
    }

    
    protected void uncaughtException(Exception exception) {
        Thread thread = Thread.currentThread();
        thread.getUncaughtExceptionHandler().uncaughtException(thread, exception);
    }

    
    protected void handleClose(ChannelContext<?> context) {
        try {
            context.closeFromSelector();
        } catch (IOException e) {
            closeException(context, e);
        }
        assert context.isOpen() == false : "Should always be done as we are on the selector thread";
    }

    
    protected void closeException(ChannelContext<?> context, Exception exception) {
        logger.debug(() -> new ParameterizedMessage("exception while closing channel: {}", context.getChannel()), exception);
    }

    
    protected void genericChannelException(ChannelContext<?> channel, Exception exception) {
        logger.debug(() -> new ParameterizedMessage("exception while handling event for channel: {}", channel.getChannel()), exception);
    }
}
