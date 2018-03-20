

package org.elasticsearch.nio;

import java.io.IOException;
import java.nio.channels.NetworkChannel;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.util.concurrent.CompletableFuture;
import java.util.function.BiConsumer;
import java.util.function.Consumer;


public abstract class ChannelContext<S extends SelectableChannel & NetworkChannel> {

    protected final S rawChannel;
    private final Consumer<Exception> exceptionHandler;
    private final CompletableFuture<Void> closeContext = new CompletableFuture<>();
    private volatile SelectionKey selectionKey;

    ChannelContext(S rawChannel, Consumer<Exception> exceptionHandler) {
        this.rawChannel = rawChannel;
        this.exceptionHandler = exceptionHandler;
    }

    protected void register() throws IOException {
        setSelectionKey(rawChannel.register(getSelector().rawSelector(), 0));
    }

    SelectionKey getSelectionKey() {
        return selectionKey;
    }

        protected void setSelectionKey(SelectionKey selectionKey) {
        this.selectionKey = selectionKey;
    }

    
    public void closeFromSelector() throws IOException {
        if (closeContext.isDone() == false) {
            try {
                rawChannel.close();
                closeContext.complete(null);
            } catch (Exception e) {
                closeContext.completeExceptionally(e);
                throw e;
            }
        }
    }

    
    public void addCloseListener(BiConsumer<Void, Throwable> listener) {
        closeContext.whenComplete(listener);
    }

    public boolean isOpen() {
        return closeContext.isDone() == false;
    }

    void handleException(Exception e) {
        exceptionHandler.accept(e);
    }

    
    public abstract void closeChannel();

    public abstract ESSelector getSelector();

    public abstract NioChannel getChannel();

}
