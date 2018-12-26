

package org.elasticsearch.index.translog;

import org.elasticsearch.common.io.stream.ByteBufferStreamInput;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Path;


public abstract class BaseTranslogReader implements Comparable<BaseTranslogReader> {

    protected final long generation;
    protected final FileChannel channel;
    protected final Path path;
    protected final long firstOperationOffset;

    public BaseTranslogReader(long generation, FileChannel channel, Path path, long firstOperationOffset) {
        assert Translog.parseIdFromFileName(path) == generation : "generation mismatch. Path: " + Translog.parseIdFromFileName(path) + " but generation: " + generation;

        this.generation = generation;
        this.path = path;
        this.channel = channel;
        this.firstOperationOffset = firstOperationOffset;
    }

    public long getGeneration() {
        return this.generation;
    }

    public abstract long sizeInBytes();

    public abstract int totalOperations();

    abstract Checkpoint getCheckpoint();

    public final long getFirstOperationOffset() {
        return firstOperationOffset;
    }

    
    protected final int readSize(ByteBuffer reusableBuffer, long position) throws IOException {
                assert reusableBuffer.capacity() >= 4 : "reusable buffer must have capacity >=4 when reading opSize. got [" + reusableBuffer.capacity() + "]";
        reusableBuffer.clear();
        reusableBuffer.limit(4);
        readBytes(reusableBuffer, position);
        reusableBuffer.flip();
                final int size = reusableBuffer.getInt() + 4;
        final long maxSize = sizeInBytes() - position;
        if (size < 0 || size > maxSize) {
            throw new TranslogCorruptedException("operation size is corrupted must be [0.." + maxSize + "] but was: " + size);
        }
        return size;
    }

    public TranslogSnapshot newSnapshot() {
        return new TranslogSnapshot(this, sizeInBytes());
    }

    
    protected final BufferedChecksumStreamInput checksummedStream(ByteBuffer reusableBuffer, long position, int opSize, BufferedChecksumStreamInput reuse) throws IOException {
        final ByteBuffer buffer;
        if (reusableBuffer.capacity() >= opSize) {
            buffer = reusableBuffer;
        } else {
            buffer = ByteBuffer.allocate(opSize);
        }
        buffer.clear();
        buffer.limit(opSize);
        readBytes(buffer, position);
        buffer.flip();
        return new BufferedChecksumStreamInput(new ByteBufferStreamInput(buffer), reuse);
    }

    protected Translog.Operation read(BufferedChecksumStreamInput inStream) throws IOException {
        return Translog.readOperation(inStream);
    }

    
    protected abstract  void readBytes(ByteBuffer buffer, long position) throws IOException;

    @Override
    public String toString() {
        return "translog [" + generation + "][" + path + "]";
    }

    @Override
    public int compareTo(BaseTranslogReader o) {
        return Long.compare(getGeneration(), o.getGeneration());
    }


    public Path path() {
        return path;
    }

    public long getLastModifiedTime() throws IOException {
        return Files.getLastModifiedTime(path).toMillis();
    }
}
