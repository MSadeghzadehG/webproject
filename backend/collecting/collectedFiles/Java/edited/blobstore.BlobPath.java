

package org.elasticsearch.common.blobstore;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public class BlobPath implements Iterable<String> {

    private static final String SEPARATOR = "/";

    private final List<String> paths;

    public BlobPath() {
        this.paths = Collections.emptyList();
    }

    public static BlobPath cleanPath() {
        return new BlobPath();
    }

    private BlobPath(List<String> paths) {
        this.paths = paths;
    }

    @Override
    public Iterator<String> iterator() {
        return paths.iterator();
    }

    public String[] toArray() {
        return paths.toArray(new String[paths.size()]);
    }

    public BlobPath add(String path) {
        List<String> paths = new ArrayList<>(this.paths);
        paths.add(path);
        return new BlobPath(Collections.unmodifiableList(paths));
    }

    public String buildAsString() {
        String p = String.join(SEPARATOR, paths);
        if (p.isEmpty() || p.endsWith(SEPARATOR)) {
            return p;
        }
        return p + SEPARATOR;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (String path : paths) {
            sb.append('[').append(path).append(']');
        }
        return sb.toString();
    }
}
