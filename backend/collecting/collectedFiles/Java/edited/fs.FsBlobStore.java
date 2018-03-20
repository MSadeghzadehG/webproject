

package org.elasticsearch.common.blobstore.fs;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.blobstore.BlobContainer;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.BlobStore;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.ByteSizeUnit;
import org.elasticsearch.common.unit.ByteSizeValue;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class FsBlobStore extends AbstractComponent implements BlobStore {

    private final Path path;

    private final int bufferSizeInBytes;

    private final boolean readOnly;

    public FsBlobStore(Settings settings, Path path) throws IOException {
        super(settings);
        this.path = path;
        this.readOnly = settings.getAsBoolean("readonly", false);
        if (!this.readOnly) {
            Files.createDirectories(path);
        }
        this.bufferSizeInBytes = (int) settings.getAsBytesSize("repositories.fs.buffer_size", new ByteSizeValue(100, ByteSizeUnit.KB)).getBytes();
    }

    @Override
    public String toString() {
        return path.toString();
    }

    public Path path() {
        return path;
    }

    public int bufferSizeInBytes() {
        return this.bufferSizeInBytes;
    }

    @Override
    public BlobContainer blobContainer(BlobPath path) {
        try {
            return new FsBlobContainer(this, path, buildAndCreate(path));
        } catch (IOException ex) {
            throw new ElasticsearchException("failed to create blob container", ex);
        }
    }

    @Override
    public void delete(BlobPath path) throws IOException {
        IOUtils.rm(buildPath(path));
    }

    @Override
    public void close() {
            }

    private synchronized Path buildAndCreate(BlobPath path) throws IOException {
        Path f = buildPath(path);
        if (!readOnly) {
            Files.createDirectories(f);
        }
        return f;
    }

    private Path buildPath(BlobPath path) {
        String[] paths = path.toArray();
        if (paths.length == 0) {
            return path();
        }
        Path blobPath = this.path.resolve(paths[0]);
        if (paths.length > 1) {
            for (int i = 1; i < paths.length; i++) {
                blobPath = blobPath.resolve(paths[i]);
            }
        }
        return blobPath;
    }
}
