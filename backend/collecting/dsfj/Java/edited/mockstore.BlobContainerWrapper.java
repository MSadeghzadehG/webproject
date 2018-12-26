
package org.elasticsearch.snapshots.mockstore;

import org.elasticsearch.common.blobstore.BlobContainer;
import org.elasticsearch.common.blobstore.BlobMetaData;
import org.elasticsearch.common.blobstore.BlobPath;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

public class BlobContainerWrapper implements BlobContainer {
    private BlobContainer delegate;

    public BlobContainerWrapper(BlobContainer delegate) {
        this.delegate = delegate;
    }

    @Override
    public BlobPath path() {
        return delegate.path();
    }

    @Override
    public boolean blobExists(String blobName) {
        return delegate.blobExists(blobName);
    }

    @Override
    public InputStream readBlob(String name) throws IOException {
        return delegate.readBlob(name);
    }

    @Override
    public void writeBlob(String blobName, InputStream inputStream, long blobSize) throws IOException {
        delegate.writeBlob(blobName, inputStream, blobSize);
    }

    @Override
    public void deleteBlob(String blobName) throws IOException {
        delegate.deleteBlob(blobName);
    }

    @Override
    public Map<String, BlobMetaData> listBlobs() throws IOException {
        return delegate.listBlobs();
    }

    @Override
    public Map<String, BlobMetaData> listBlobsByPrefix(String blobNamePrefix) throws IOException {
        return delegate.listBlobsByPrefix(blobNamePrefix);
    }

    @Override
    public void move(String sourceBlobName, String targetBlobName) throws IOException {
        delegate.move(sourceBlobName, targetBlobName);
    }
}
