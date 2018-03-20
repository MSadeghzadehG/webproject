

package org.elasticsearch.common.lucene.store;

import org.apache.lucene.store.IndexOutput;

import java.io.IOException;
import java.io.OutputStream;


public class IndexOutputOutputStream extends OutputStream {

    private final IndexOutput out;

    public IndexOutputOutputStream(IndexOutput out) {
        this.out = out;
    }

    @Override
    public void write(int b) throws IOException {
        out.writeByte((byte) b);
    }

    @Override
    public void write(byte[] b) throws IOException {
        out.writeBytes(b, b.length);
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        out.writeBytes(b, off, len);
    }

    @Override
    public void close() throws IOException {
        out.close();
    }
}
