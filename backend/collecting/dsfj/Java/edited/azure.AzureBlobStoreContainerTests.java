

package org.elasticsearch.repositories.azure;

import com.microsoft.azure.storage.StorageException;
import org.elasticsearch.cluster.metadata.RepositoryMetaData;
import org.elasticsearch.common.blobstore.BlobStore;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.repositories.ESBlobStoreContainerTestCase;

import java.io.IOException;
import java.net.URISyntaxException;

public class AzureBlobStoreContainerTests extends ESBlobStoreContainerTestCase {
    @Override
    protected BlobStore newBlobStore() throws IOException {
        try {
            RepositoryMetaData repositoryMetaData = new RepositoryMetaData("azure", "ittest", Settings.EMPTY);
            AzureStorageServiceMock client = new AzureStorageServiceMock();
            return new AzureBlobStore(repositoryMetaData, Settings.EMPTY, client);
        } catch (URISyntaxException | StorageException e) {
            throw new IOException(e);
        }
    }
}
