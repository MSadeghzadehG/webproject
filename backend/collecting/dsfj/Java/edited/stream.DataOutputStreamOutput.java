

package org.elasticsearch.common.io.stream;

import java.io.Closeable;
import java.io.DataOutput;
import java.io.IOException;

public class DataOutputStreamOutput extends StreamOutput {

    private final DataOutput out;

    public DataOutputStreamOutput(DataOutput out) {
        this.out = out;
    }

    @Override
    public void writeByte(byte b) throws IOException {
        out.writeByte(b);
    }

    @Override
    public void writeBytes(byte[] b, int offset, int length) throws IOException {
        out.write(b, offset, length);
    }

    @Override
    public void flush() throws IOException {
            }

    @Override
    public void reset() throws IOException {
            }

    @Override
    public void close() throws IOException {
        if (out instanceof Closeable) {
            ((Closeable) out).close();
        }
    }
}