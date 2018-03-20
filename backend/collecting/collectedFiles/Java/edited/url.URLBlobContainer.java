

package org.elasticsearch.common.blobstore.url;

import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.blobstore.BlobMetaData;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.support.AbstractBlobContainer;

import java.io.BufferedInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.nio.file.NoSuchFileException;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.Map;


public class URLBlobContainer extends AbstractBlobContainer {

    protected final URLBlobStore blobStore;

    protected final URL path;

    
    public URLBlobContainer(URLBlobStore blobStore, BlobPath blobPath, URL path) {
        super(blobPath);
        this.blobStore = blobStore;
        this.path = path;
    }

    
    public URL url() {
        return this.path;
    }

    
    @Override
    public Map<String, BlobMetaData> listBlobs() throws IOException {
        throw new UnsupportedOperationException("URL repository doesn't support this operation");
    }

    
    @Override
    public Map<String, BlobMetaData> listBlobsByPrefix(String blobNamePrefix) throws IOException {
        throw new UnsupportedOperationException("URL repository doesn't support this operation");
    }

    @Override
    public void move(String from, String to) throws IOException {
        throw new UnsupportedOperationException("URL repository doesn't support this operation");
    }

    
    @Override
    public void deleteBlob(String blobName) throws IOException {
        throw new UnsupportedOperationException("URL repository is read only");
    }

    
    @Override
    public boolean blobExists(String blobName) {
        throw new UnsupportedOperationException("URL repository doesn't support this operation");
    }

    @Override
    public InputStream readBlob(String name) throws IOException {
        try {
            return new BufferedInputStream(getInputStream(new URL(path, name)), blobStore.bufferSizeInBytes());
        } catch (FileNotFoundException fnfe) {
            throw new NoSuchFileException("[" + name + "] blob not found");
        }
    }

    @Override
    public void writeBlob(String blobName, InputStream inputStream, long blobSize) throws IOException {
        throw new UnsupportedOperationException("URL repository doesn't support this operation");
    }

    @SuppressForbidden(reason = "We call connect in doPrivileged and provide SocketPermission")
    private static InputStream getInputStream(URL url) throws IOException {
        try {
            return AccessController.doPrivileged((PrivilegedExceptionAction<InputStream>) url::openStream);
        } catch (PrivilegedActionException e) {
            throw (IOException) e.getCause();
        }
    }

}
