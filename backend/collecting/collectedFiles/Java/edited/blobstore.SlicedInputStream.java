
package org.elasticsearch.index.snapshots.blobstore;

import org.elasticsearch.core.internal.io.IOUtils;

import java.io.IOException;
import java.io.InputStream;


public abstract class SlicedInputStream extends InputStream {
    private long slice = 0;
    private InputStream currentStream;
    private final long numSlices;
    private boolean initialized = false;

    
    protected SlicedInputStream(final long numSlices) {
        this.numSlices = numSlices;
    }

    private InputStream nextStream() throws IOException {
        assert initialized == false || currentStream != null;
        initialized = true;
        IOUtils.close(currentStream);
        if (slice < numSlices) {
            currentStream = openSlice(slice++);
        } else {
            currentStream = null;
        }
        return currentStream;
    }

    
    protected abstract InputStream openSlice(long slice) throws IOException;

    private InputStream currentStream() throws IOException {
        if (currentStream == null) {
            return initialized ? null : nextStream();
        }
        return currentStream;
    }

    @Override
    public final int read() throws IOException {
        InputStream stream = currentStream();
        if (stream == null) {
            return -1;
        }
        final int read = stream.read();
        if (read == -1) {
            nextStream();
            return read();
        }
        return read;
    }

    @Override
    public final int read(byte[] buffer, int offset, int length) throws IOException {
        final InputStream stream = currentStream();
        if (stream == null) {
            return -1;
        }
        final int read = stream.read(buffer, offset, length);
        if (read <= 0) {
            nextStream();
            return read(buffer, offset, length);
        }
        return read;
    }

    @Override
    public final void close() throws IOException {
        IOUtils.close(currentStream);
        initialized = true;
        currentStream = null;
    }

    @Override
    public final int available() throws IOException {
        InputStream stream = currentStream();
        return stream == null ? 0 : stream.available();
    }

}
