

package org.elasticsearch.repositories.azure;

import com.microsoft.azure.storage.LocationMode;
import com.microsoft.azure.storage.StorageException;
import org.apache.logging.log4j.Logger;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.blobstore.BlobMetaData;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.support.AbstractBlobContainer;
import org.elasticsearch.common.logging.Loggers;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URISyntaxException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.NoSuchFileException;
import java.util.Map;

public class AzureBlobContainer extends AbstractBlobContainer {

    private final Logger logger = Loggers.getLogger(AzureBlobContainer.class);
    private final AzureBlobStore blobStore;

    private final String keyPath;

    public AzureBlobContainer(BlobPath path, AzureBlobStore blobStore) {
        super(path);
        this.blobStore = blobStore;
        this.keyPath = path.buildAsString();
    }

    @Override
    public boolean blobExists(String blobName) {
        logger.trace("blobExists({})", blobName);
        try {
            return blobStore.blobExists(buildKey(blobName));
        } catch (URISyntaxException | StorageException e) {
            logger.warn("can not access [{}] in container {{}}: {}", blobName, blobStore, e.getMessage());
        }
        return false;
    }

    @Override
    public InputStream readBlob(String blobName) throws IOException {
        logger.trace("readBlob({})", blobName);

        if (blobStore.getLocationMode() == LocationMode.SECONDARY_ONLY && !blobExists(blobName)) {
                                                                                    throw new NoSuchFileException("Blob [" + blobName + "] does not exist");
        }

        try {
            return blobStore.getInputStream(buildKey(blobName));
        } catch (StorageException e) {
            if (e.getHttpStatusCode() == HttpURLConnection.HTTP_NOT_FOUND) {
                throw new NoSuchFileException(e.getMessage());
            }
            throw new IOException(e);
        } catch (URISyntaxException e) {
            throw new IOException(e);
        }
    }

    @Override
    public void writeBlob(String blobName, InputStream inputStream, long blobSize) throws IOException {
        if (blobExists(blobName)) {
            throw new FileAlreadyExistsException("blob [" + blobName + "] already exists, cannot overwrite");
        }
        logger.trace("writeBlob({}, stream, {})", buildKey(blobName), blobSize);
        try {
            blobStore.writeBlob(buildKey(blobName), inputStream, blobSize);
        } catch (URISyntaxException|StorageException e) {
            throw new IOException("Can not write blob " + blobName, e);
        }
    }

    @Override
    public void deleteBlob(String blobName) throws IOException {
        logger.trace("deleteBlob({})", blobName);

        if (!blobExists(blobName)) {
            throw new NoSuchFileException("Blob [" + blobName + "] does not exist");
        }

        try {
            blobStore.deleteBlob(buildKey(blobName));
        } catch (URISyntaxException | StorageException e) {
            logger.warn("can not access [{}] in container {{}}: {}", blobName, blobStore, e.getMessage());
            throw new IOException(e);
        }
    }

    @Override
    public Map<String, BlobMetaData> listBlobsByPrefix(@Nullable String prefix) throws IOException {
        logger.trace("listBlobsByPrefix({})", prefix);

        try {
            return blobStore.listBlobsByPrefix(keyPath, prefix);
        } catch (URISyntaxException | StorageException e) {
            logger.warn("can not access [{}] in container {{}}: {}", prefix, blobStore, e.getMessage());
            throw new IOException(e);
        }
    }

    @Override
    public void move(String sourceBlobName, String targetBlobName) throws IOException {
        logger.trace("move({}, {})", sourceBlobName, targetBlobName);
        try {
            String source = keyPath + sourceBlobName;
            String target = keyPath + targetBlobName;

            logger.debug("moving blob [{}] to [{}] in container {{}}", source, target, blobStore);

            blobStore.moveBlob(source, target);
        } catch (URISyntaxException | StorageException e) {
            logger.warn("can not move blob [{}] to [{}] in container {{}}: {}", sourceBlobName, targetBlobName, blobStore, e.getMessage());
            throw new IOException(e);
        }
    }

    @Override
    public Map<String, BlobMetaData> listBlobs() throws IOException {
        logger.trace("listBlobs()");
        return listBlobsByPrefix(null);
    }

    protected String buildKey(String blobName) {
        return keyPath + (blobName == null ? "" : blobName);
    }
}
