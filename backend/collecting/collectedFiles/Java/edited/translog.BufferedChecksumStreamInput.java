

package org.elasticsearch.index.translog;

import org.apache.lucene.store.BufferedChecksum;
import org.elasticsearch.common.io.stream.FilterStreamInput;
import org.elasticsearch.common.io.stream.StreamInput;

import java.io.EOFException;
import java.io.IOException;
import java.util.zip.CRC32;
import java.util.zip.Checksum;


public final class BufferedChecksumStreamInput extends FilterStreamInput {
    private static final int SKIP_BUFFER_SIZE = 1024;
    private byte[] skipBuffer;
    private final Checksum digest;

    public BufferedChecksumStreamInput(StreamInput in) {
        super(in);
        this.digest = new BufferedChecksum(new CRC32());
    }

    public BufferedChecksumStreamInput(StreamInput in, BufferedChecksumStreamInput reuse) {
        super(in);
        if (reuse == null ) {
            this.digest = new BufferedChecksum(new CRC32());
        } else {
            this.digest = reuse.digest;
            digest.reset();
            this.skipBuffer = reuse.skipBuffer;
        }
    }

    public long getChecksum() {
        return this.digest.getValue();
    }

    @Override
    public byte readByte() throws IOException {
        final byte b = delegate.readByte();
        digest.update(b);
        return b;
    }

    @Override
    public void readBytes(byte[] b, int offset, int len) throws IOException {
        delegate.readBytes(b, offset, len);
        digest.update(b, offset, len);
    }

    @Override
    public void reset() throws IOException {
        delegate.reset();
        digest.reset();
    }

    @Override
    public int read() throws IOException {
        return readByte() & 0xFF;
    }

    @Override
    public boolean markSupported() {
        return delegate.markSupported();
    }


    @Override
    public long skip(long numBytes) throws IOException {
        if (numBytes < 0) {
            throw new IllegalArgumentException("numBytes must be >= 0, got " + numBytes);
        }
        if (skipBuffer == null) {
            skipBuffer = new byte[SKIP_BUFFER_SIZE];
        }
        assert skipBuffer.length == SKIP_BUFFER_SIZE;
        long skipped = 0;
        for (; skipped < numBytes; ) {
            final int step = (int) Math.min(SKIP_BUFFER_SIZE, numBytes - skipped);
            readBytes(skipBuffer, 0, step);
            skipped += step;
        }
        return skipped;
    }


    @Override
    public synchronized void mark(int readlimit) {
        delegate.mark(readlimit);
    }

    public void resetDigest() {
        digest.reset();
    }

}
