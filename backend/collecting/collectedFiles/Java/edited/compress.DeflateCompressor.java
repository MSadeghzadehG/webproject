

package org.elasticsearch.common.compress;

import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.io.stream.InputStreamStreamInput;
import org.elasticsearch.common.io.stream.OutputStreamStreamOutput;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;
import java.util.zip.InflaterInputStream;


public class DeflateCompressor implements Compressor {

                    private static final byte[] HEADER = new byte[]{'D', 'F', 'L', '\0'};
        private static final int LEVEL = 3;
            private static final int BUFFER_SIZE = 4096;

    @Override
    public boolean isCompressed(BytesReference bytes) {
        if (bytes.length() < HEADER.length) {
            return false;
        }
        for (int i = 0; i < HEADER.length; ++i) {
            if (bytes.get(i) != HEADER[i]) {
                return false;
            }
        }
        return true;
    }

    @Override
    public StreamInput streamInput(StreamInput in) throws IOException {
        final byte[] headerBytes = new byte[HEADER.length];
        int len = 0;
        while (len < headerBytes.length) {
            final int read = in.read(headerBytes, len, headerBytes.length - len);
            if (read == -1) {
                break;
            }
            len += read;
        }
        if (len != HEADER.length || Arrays.equals(headerBytes, HEADER) == false) {
            throw new IllegalArgumentException("Input stream is not compressed with DEFLATE!");
        }

        final boolean nowrap = true;
        final Inflater inflater = new Inflater(nowrap);
        InputStream decompressedIn = new InflaterInputStream(in, inflater, BUFFER_SIZE);
        decompressedIn = new BufferedInputStream(decompressedIn, BUFFER_SIZE);
        return new InputStreamStreamInput(decompressedIn) {
            final AtomicBoolean closed = new AtomicBoolean(false);

            public void close() throws IOException {
                try {
                    super.close();
                } finally {
                    if (closed.compareAndSet(false, true)) {
                                                inflater.end();
                    }
                }
            }
        };
    }

    @Override
    public StreamOutput streamOutput(StreamOutput out) throws IOException {
        out.writeBytes(HEADER);
        final boolean nowrap = true;
        final Deflater deflater = new Deflater(LEVEL, nowrap);
        final boolean syncFlush = true;
        DeflaterOutputStream deflaterOutputStream = new DeflaterOutputStream(out, deflater, BUFFER_SIZE, syncFlush);
        OutputStream compressedOut = new BufferedOutputStream(deflaterOutputStream, BUFFER_SIZE);
        return new OutputStreamStreamOutput(compressedOut) {
            final AtomicBoolean closed = new AtomicBoolean(false);

            public void close() throws IOException {
                try {
                    super.close();
                } finally {
                    if (closed.compareAndSet(false, true)) {
                                                deflater.end();
                    }
                }
            }
        };
    }
}
