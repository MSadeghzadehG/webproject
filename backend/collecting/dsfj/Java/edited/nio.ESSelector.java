

package org.elasticsearch.nio;

import java.io.Closeable;
import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.Collectors;


public abstract class ESSelector implements Closeable {

    final Selector selector;
    final ConcurrentLinkedQueue<ChannelContext<?>> channelsToClose = new ConcurrentLinkedQueue<>();

    private final EventHandler eventHandler;
    private final ReentrantLock runLock = new ReentrantLock();
    private final CountDownLatch exitedLoop = new CountDownLatch(1);
    private final AtomicBoolean isClosed = new AtomicBoolean(false);
    private final CompletableFuture<Void> isRunningFuture = new CompletableFuture<>();
    private volatile Thread thread;

    ESSelector(EventHandler eventHandler) throws IOException {
        this(eventHandler, Selector.open());
    }

    ESSelector(EventHandler eventHandler, Selector selector) {
        this.eventHandler = eventHandler;
        this.selector = selector;
    }

    
    public void runLoop() {
        if (runLock.tryLock()) {
            isRunningFuture.complete(null);
            try {
                setThread();
                while (isOpen()) {
                    singleLoop();
                }
            } finally {
                try {
                    cleanupAndCloseChannels();
                } finally {
                    try {
                        selector.close();
                    } catch (IOException e) {
                        eventHandler.closeSelectorException(e);
                    } finally {
                        runLock.unlock();
                        exitedLoop.countDown();
                    }
                }
            }
        } else {
            throw new IllegalStateException("selector is already running");
        }
    }

    void singleLoop() {
        try {
            closePendingChannels();
            preSelect();

            int ready = selector.select(300);
            if (ready > 0) {
                Set<SelectionKey> selectionKeys = selector.selectedKeys();
                Iterator<SelectionKey> keyIterator = selectionKeys.iterator();
                while (keyIterator.hasNext()) {
                    SelectionKey sk = keyIterator.next();
                    keyIterator.remove();
                    if (sk.isValid()) {
                        try {
                            processKey(sk);
                        } catch (CancelledKeyException cke) {
                            eventHandler.genericChannelException((ChannelContext<?>) sk.attachment(),  cke);
                        }
                    } else {
                        eventHandler.genericChannelException((ChannelContext<?>) sk.attachment(),  new CancelledKeyException());
                    }
                }
            }
        } catch (ClosedSelectorException e) {
            if (isOpen()) {
                throw e;
            }
        } catch (IOException e) {
            eventHandler.selectException(e);
        } catch (Exception e) {
            eventHandler.uncaughtException(e);
        }
    }

    void cleanupAndCloseChannels() {
        cleanup();
        channelsToClose.addAll(selector.keys().stream().map(sk -> (ChannelContext<?>) sk.attachment()).collect(Collectors.toList()));
        closePendingChannels();
    }

    
    abstract void processKey(SelectionKey selectionKey) throws CancelledKeyException;

    
    abstract void preSelect();

    
    abstract void cleanup();

    void setThread() {
        thread = Thread.currentThread();
    }

    public boolean isOnCurrentThread() {
        return Thread.currentThread() == thread;
    }

    public void assertOnSelectorThread() {
        assert isOnCurrentThread() : "Must be on selector thread to perform this operation. Currently on thread ["
            + Thread.currentThread().getName() + "].";
    }

    void wakeup() {
                selector.wakeup();
    }

    @Override
    public void close() throws IOException {
        if (isClosed.compareAndSet(false, true)) {
            wakeup();
            if (isRunning()) {
                try {
                    exitedLoop.await();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new IllegalStateException("Thread was interrupted while waiting for selector to close", e);
                }
            } else if (selector.isOpen()) {
                selector.close();
            }
        }
    }

    public void queueChannelClose(NioChannel channel) {
        ChannelContext<?> context = channel.getContext();
        assert context.getSelector() == this : "Must schedule a channel for closure with its selector";
        channelsToClose.offer(context);
        ensureSelectorOpenForEnqueuing(channelsToClose, context);
        wakeup();
    }

    public Selector rawSelector() {
        return selector;
    }

    public boolean isOpen() {
        return isClosed.get() == false;
    }

    public boolean isRunning() {
        return runLock.isLocked();
    }

    public Future<Void> isRunningFuture() {
        return isRunningFuture;
    }

    
    <O> void ensureSelectorOpenForEnqueuing(ConcurrentLinkedQueue<O> queue, O objectAdded) {
        if (isOpen() == false && isOnCurrentThread() == false) {
            if (queue.remove(objectAdded)) {
                throw new IllegalStateException("selector is already closed");
            }
        }
    }

    private void closePendingChannels() {
        ChannelContext<?> channelContext;
        while ((channelContext = channelsToClose.poll()) != null) {
            eventHandler.handleClose(channelContext);
        }
    }
}
