

package org.elasticsearch.index.snapshots.blobstore;

import org.apache.lucene.store.RateLimiter;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


public class RateLimitingInputStream extends FilterInputStream {

    private final RateLimiter rateLimiter;

    private final Listener listener;

    private long bytesSinceLastRateLimit;

    public interface Listener {
        void onPause(long nanos);
    }

    public RateLimitingInputStream(InputStream delegate, RateLimiter rateLimiter, Listener listener) {
        super(delegate);
        this.rateLimiter = rateLimiter;
        this.listener = listener;
    }

    private void maybePause(int bytes) throws IOException {
        bytesSinceLastRateLimit += bytes;
        if (bytesSinceLastRateLimit >= rateLimiter.getMinPauseCheckBytes()) {
            long pause = rateLimiter.pause(bytesSinceLastRateLimit);
            bytesSinceLastRateLimit = 0;
            if (pause > 0) {
                listener.onPause(pause);
            }
        }
    }

    @Override
    public int read() throws IOException {
        int b = super.read();
        maybePause(1);
        return b;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        int n = super.read(b, off, len);
        if (n > 0) {
            maybePause(n);
        }
        return n;
    }
}
