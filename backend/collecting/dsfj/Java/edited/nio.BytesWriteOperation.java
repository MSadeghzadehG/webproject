

package org.elasticsearch.nio;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.function.BiConsumer;

public class BytesWriteOperation implements WriteOperation {

    private final SocketChannelContext channelContext;
    private final BiConsumer<Void, Throwable> listener;
    private final ByteBuffer[] buffers;
    private final int[] offsets;
    private final int length;
    private int internalIndex;

    public BytesWriteOperation(SocketChannelContext channelContext, ByteBuffer[] buffers, BiConsumer<Void, Throwable> listener) {
        this.channelContext = channelContext;
        this.listener = listener;
        this.buffers = buffers;
        this.offsets = new int[buffers.length];
        int offset = 0;
        for (int i = 0; i < buffers.length; i++) {
            ByteBuffer buffer = buffers[i];
            offsets[i] = offset;
            offset += buffer.remaining();
        }
        length = offset;
    }

    @Override
    public BiConsumer<Void, Throwable> getListener() {
        return listener;
    }

    @Override
    public SocketChannelContext getChannel() {
        return channelContext;
    }

    public boolean isFullyFlushed() {
        assert length >= internalIndex : "Should never have an index that is greater than the length [length=" + length + ", index="
            + internalIndex + "]";
        return internalIndex == length;
    }

    public void incrementIndex(int delta) {
        internalIndex += delta;
        assert length >= internalIndex : "Should never increment index past length [length=" + length + ", post-increment index="
            + internalIndex + ", delta=" + delta + "]";
    }

    public ByteBuffer[] getBuffersToWrite() {
        final int index = Arrays.binarySearch(offsets, internalIndex);
        int offsetIndex = index < 0 ? (-(index + 1)) - 1 : index;

        ByteBuffer[] postIndexBuffers = new ByteBuffer[buffers.length - offsetIndex];

        ByteBuffer firstBuffer = buffers[offsetIndex].duplicate();
        firstBuffer.position(internalIndex - offsets[offsetIndex]);
        postIndexBuffers[0] = firstBuffer;
        int j = 1;
        for (int i = (offsetIndex + 1); i < buffers.length; ++i) {
            postIndexBuffers[j++] = buffers[i].duplicate();
        }

        return postIndexBuffers;
    }

}
