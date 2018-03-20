

package org.elasticsearch.nio;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.BiConsumer;
import java.util.function.Consumer;

public class BytesChannelContext extends SocketChannelContext {

    private final ReadConsumer readConsumer;
    private final InboundChannelBuffer channelBuffer;
    private final LinkedList<BytesWriteOperation> queued = new LinkedList<>();
    private final AtomicBoolean isClosing = new AtomicBoolean(false);

    public BytesChannelContext(NioSocketChannel channel, SocketSelector selector, Consumer<Exception> exceptionHandler,
                               ReadConsumer readConsumer, InboundChannelBuffer channelBuffer) {
        super(channel, selector, exceptionHandler);
        this.readConsumer = readConsumer;
        this.channelBuffer = channelBuffer;
    }

    @Override
    public int read() throws IOException {
        if (channelBuffer.getRemaining() == 0) {
                        channelBuffer.ensureCapacity(channelBuffer.getCapacity() + 1);
        }

        int bytesRead = readFromChannel(channelBuffer.sliceBuffersFrom(channelBuffer.getIndex()));

        if (bytesRead == 0) {
            return 0;
        }

        channelBuffer.incrementIndex(bytesRead);

        int bytesConsumed = Integer.MAX_VALUE;
        while (bytesConsumed > 0 && channelBuffer.getIndex() > 0) {
            bytesConsumed = readConsumer.consumeReads(channelBuffer);
            channelBuffer.release(bytesConsumed);
        }

        return bytesRead;
    }

    @Override
    public void sendMessage(ByteBuffer[] buffers, BiConsumer<Void, Throwable> listener) {
        if (isClosing.get()) {
            listener.accept(null, new ClosedChannelException());
            return;
        }

        BytesWriteOperation writeOperation = new BytesWriteOperation(this, buffers, listener);
        SocketSelector selector = getSelector();
        if (selector.isOnCurrentThread() == false) {
            selector.queueWrite(writeOperation);
            return;
        }

        selector.queueWriteInChannelBuffer(writeOperation);
    }

    @Override
    public void queueWriteOperation(WriteOperation writeOperation) {
        getSelector().assertOnSelectorThread();
        queued.add((BytesWriteOperation) writeOperation);
    }

    @Override
    public void flushChannel() throws IOException {
        getSelector().assertOnSelectorThread();
        int ops = queued.size();
        if (ops == 1) {
            singleFlush(queued.pop());
        } else if (ops > 1) {
            multiFlush();
        }
    }

    @Override
    public boolean hasQueuedWriteOps() {
        getSelector().assertOnSelectorThread();
        return queued.isEmpty() == false;
    }

    @Override
    public void closeChannel() {
        if (isClosing.compareAndSet(false, true)) {
            getSelector().queueChannelClose(channel);
        }
    }

    @Override
    public boolean selectorShouldClose() {
        return isPeerClosed() || hasIOException() || isClosing.get();
    }

    @Override
    public void closeFromSelector() throws IOException {
        getSelector().assertOnSelectorThread();
        if (channel.isOpen()) {
            IOException channelCloseException = null;
            try {
                super.closeFromSelector();
            } catch (IOException e) {
                channelCloseException = e;
            }
                        isClosing.set(true);
            channelBuffer.close();
            for (BytesWriteOperation op : queued) {
                getSelector().executeFailedListener(op.getListener(), new ClosedChannelException());
            }
            queued.clear();
            if (channelCloseException != null) {
                throw channelCloseException;
            }
        }
    }

    private void singleFlush(BytesWriteOperation headOp) throws IOException {
        try {
            int written = flushToChannel(headOp.getBuffersToWrite());
            headOp.incrementIndex(written);
        } catch (IOException e) {
            getSelector().executeFailedListener(headOp.getListener(), e);
            throw e;
        }

        if (headOp.isFullyFlushed()) {
            getSelector().executeListener(headOp.getListener(), null);
        } else {
            queued.push(headOp);
        }
    }

    private void multiFlush() throws IOException {
        boolean lastOpCompleted = true;
        while (lastOpCompleted && queued.isEmpty() == false) {
            BytesWriteOperation op = queued.pop();
            singleFlush(op);
            lastOpCompleted = op.isFullyFlushed();
        }
    }
}
