

package org.elasticsearch.plugins;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


abstract class ProgressInputStream extends FilterInputStream {

    private final int expectedTotalSize;
    private int currentPercent;
    private int count = 0;

    ProgressInputStream(InputStream is, int expectedTotalSize) {
        super(is);
        this.expectedTotalSize = expectedTotalSize;
        this.currentPercent = 0;
    }

    @Override
    public int read() throws IOException {
        int read = in.read();
        checkProgress(read == -1 ? -1 : 1);
        return read;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        int byteCount = super.read(b, off, len);
        checkProgress(byteCount);
        return byteCount;
    }

    @Override
    public int read(byte b[]) throws IOException {
        return read(b, 0, b.length);
    }

    void checkProgress(int byteCount) {
                if (byteCount == -1) {
            currentPercent = 100;
            onProgress(currentPercent);
        } else {
            count += byteCount;
                                    int percent = Math.min(99, (int) Math.floor(100.0*count/expectedTotalSize));
            if (percent > currentPercent) {
                currentPercent = percent;
                onProgress(percent);
            }
        }
    }

    public void onProgress(int percent) {}
}
