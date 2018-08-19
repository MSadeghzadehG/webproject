
package org.elasticsearch.xpack.sql.client.shared;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Bytes {

    private final byte[] buf;
    private final int size;

    public Bytes(byte[] buf, int size) {
        this.buf = buf;
        this.size = size;
    }

    public byte[] bytes() {
        return buf;
    }

    public int size() {
        return size;
    }

    public byte[] copy() {
        return Arrays.copyOf(buf, size);
    }

    public String toString() {
        return new String(buf, 0, size, StandardCharsets.UTF_8);
    }
}
