

package org.elasticsearch.repositories.gcs;

import com.google.api.client.googleapis.batch.BatchRequest;
import com.google.api.client.googleapis.batch.json.JsonBatchCallback;
import com.google.api.client.googleapis.json.GoogleJsonError;
import com.google.api.client.googleapis.json.GoogleJsonResponseException;
import com.google.api.client.http.HttpHeaders;
import com.google.api.client.http.InputStreamContent;
import com.google.api.services.storage.Storage;
import com.google.api.services.storage.model.Bucket;
import com.google.api.services.storage.model.Objects;
import com.google.api.services.storage.model.StorageObject;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.blobstore.BlobContainer;
import org.elasticsearch.common.blobstore.BlobMetaData;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.BlobStore;
import org.elasticsearch.common.blobstore.BlobStoreException;
import org.elasticsearch.common.blobstore.support.PlainBlobMetaData;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.concurrent.CountDown;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.NoSuchFileException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Spliterator;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import static java.net.HttpURLConnection.HTTP_NOT_FOUND;

class GoogleCloudStorageBlobStore extends AbstractComponent implements BlobStore {

    
    private static final int MAX_BATCHING_REQUESTS = 999;

    private final Storage client;
    private final String bucket;

    GoogleCloudStorageBlobStore(Settings settings, String bucket, Storage storageClient) {
        super(settings);
        this.bucket = bucket;
        this.client = storageClient;

        if (doesBucketExist(bucket) == false) {
            throw new BlobStoreException("Bucket [" + bucket + "] does not exist");
        }
    }

    @Override
    public BlobContainer blobContainer(BlobPath path) {
        return new GoogleCloudStorageBlobContainer(path, this);
    }

    @Override
    public void delete(BlobPath path) throws IOException {
        deleteBlobsByPrefix(path.buildAsString());
    }

    @Override
    public void close() {
    }

    
    boolean doesBucketExist(String bucketName) {
        try {
            return SocketAccess.doPrivilegedIOException(() -> {
                try {
                    Bucket bucket = client.buckets().get(bucketName).execute();
                    if (bucket != null) {
                        return Strings.hasText(bucket.getId());
                    }
                } catch (GoogleJsonResponseException e) {
                    GoogleJsonError error = e.getDetails();
                    if ((e.getStatusCode() == HTTP_NOT_FOUND) || ((error != null) && (error.getCode() == HTTP_NOT_FOUND))) {
                        return false;
                    }
                    throw e;
                }
                return false;
            });
        } catch (IOException e) {
            throw new BlobStoreException("Unable to check if bucket [" + bucketName + "] exists", e);
        }
    }

    
    Map<String, BlobMetaData> listBlobs(String path) throws IOException {
        return SocketAccess.doPrivilegedIOException(() -> listBlobsByPath(bucket, path, path));
    }

    
    Map<String, BlobMetaData> listBlobsByPrefix(String path, String prefix) throws IOException {
        return SocketAccess.doPrivilegedIOException(() -> listBlobsByPath(bucket, buildKey(path, prefix), path));
    }

    
    private Map<String, BlobMetaData> listBlobsByPath(String bucketName, String path, String pathToRemove) throws IOException {
        return blobsStream(client, bucketName, path, MAX_BATCHING_REQUESTS)
                .map(new BlobMetaDataConverter(pathToRemove))
                .collect(Collectors.toMap(PlainBlobMetaData::name, Function.identity()));
    }

    
    boolean blobExists(String blobName) throws IOException {
        try {
            StorageObject blob = SocketAccess.doPrivilegedIOException(() -> client.objects().get(bucket, blobName).execute());
            if (blob != null) {
                return Strings.hasText(blob.getId());
            }
        } catch (GoogleJsonResponseException e) {
            GoogleJsonError error = e.getDetails();
            if ((e.getStatusCode() == HTTP_NOT_FOUND) || ((error != null) && (error.getCode() == HTTP_NOT_FOUND))) {
                return false;
            }
            throw e;
        }
        return false;
    }

    
    InputStream readBlob(String blobName) throws IOException {
        try {
            return SocketAccess.doPrivilegedIOException(() -> {
                Storage.Objects.Get object = client.objects().get(bucket, blobName);
                return object.executeMediaAsInputStream();
            });
        } catch (GoogleJsonResponseException e) {
            GoogleJsonError error = e.getDetails();
            if ((e.getStatusCode() == HTTP_NOT_FOUND) || ((error != null) && (error.getCode() == HTTP_NOT_FOUND))) {
                throw new NoSuchFileException(e.getMessage());
            }
            throw e;
        }
    }

    
    void writeBlob(String blobName, InputStream inputStream, long blobSize) throws IOException {
        SocketAccess.doPrivilegedVoidIOException(() -> {
            InputStreamContent stream = new InputStreamContent(null, inputStream);
            stream.setLength(blobSize);

            Storage.Objects.Insert insert = client.objects().insert(bucket, null, stream);
            insert.setName(blobName);
            insert.execute();
        });
    }

    
    void deleteBlob(String blobName) throws IOException {
        if (!blobExists(blobName)) {
            throw new NoSuchFileException("Blob [" + blobName + "] does not exist");
        }
        SocketAccess.doPrivilegedIOException(() -> client.objects().delete(bucket, blobName).execute());
    }

    
    void deleteBlobsByPrefix(String prefix) throws IOException {
        deleteBlobs(listBlobsByPath(bucket, prefix, null).keySet());
    }

    
    void deleteBlobs(Collection<String> blobNames) throws IOException {
        if (blobNames == null || blobNames.isEmpty()) {
            return;
        }

        if (blobNames.size() == 1) {
            deleteBlob(blobNames.iterator().next());
            return;
        }
        final List<Storage.Objects.Delete> deletions = new ArrayList<>(Math.min(MAX_BATCHING_REQUESTS, blobNames.size()));
        final Iterator<String> blobs = blobNames.iterator();

        SocketAccess.doPrivilegedVoidIOException(() -> {
            while (blobs.hasNext()) {
                                deletions.add(client.objects().delete(bucket, blobs.next()));

                if (blobs.hasNext() == false || deletions.size() == MAX_BATCHING_REQUESTS) {
                    try {
                                                BatchRequest batch = client.batch();

                                                CountDown countDown = new CountDown(deletions.size());

                        for (Storage.Objects.Delete delete : deletions) {
                                                        delete.queue(batch, new JsonBatchCallback<Void>() {
                                @Override
                                public void onFailure(GoogleJsonError e, HttpHeaders responseHeaders) throws IOException {
                                    logger.error("failed to delete blob [{}] in bucket [{}]: {}", delete.getObject(), delete.getBucket(), e
                                        .getMessage());
                                }

                                @Override
                                public void onSuccess(Void aVoid, HttpHeaders responseHeaders) throws IOException {
                                    countDown.countDown();
                                }
                            });
                        }

                        batch.execute();

                        if (countDown.isCountedDown() == false) {
                            throw new IOException("Failed to delete all [" + deletions.size() + "] blobs");
                        }
                    } finally {
                        deletions.clear();
                    }
                }
            }
        });
    }

    
    void moveBlob(String sourceBlob, String targetBlob) throws IOException {
        SocketAccess.doPrivilegedIOException(() -> {
                        client.objects().copy(bucket, sourceBlob, bucket, targetBlob, null).execute();
            client.objects().delete(bucket, sourceBlob).execute();
            return null;
        });
    }

    private String buildKey(String keyPath, String s) {
        assert s != null;
        return keyPath + s;
    }

    
    class BlobMetaDataConverter implements Function<StorageObject, PlainBlobMetaData> {

        private final String pathToRemove;

        BlobMetaDataConverter(String pathToRemove) {
            this.pathToRemove = pathToRemove;
        }

        @Override
        public PlainBlobMetaData apply(StorageObject storageObject) {
            String blobName = storageObject.getName();
            if (Strings.hasLength(pathToRemove)) {
                blobName = blobName.substring(pathToRemove.length());
            }
            return new PlainBlobMetaData(blobName, storageObject.getSize().longValue());
        }
    }

    
    static class StorageObjectsSpliterator implements Spliterator<StorageObject> {

        private final Storage.Objects.List list;

        StorageObjectsSpliterator(Storage client, String bucketName, String prefix, long pageSize) throws IOException {
            list = SocketAccess.doPrivilegedIOException(() -> client.objects().list(bucketName));
            list.setMaxResults(pageSize);
            if (prefix != null) {
                list.setPrefix(prefix);
            }
        }

        @Override
        public boolean tryAdvance(Consumer<? super StorageObject> action) {
            try {
                                Objects objects = SocketAccess.doPrivilegedIOException(list::execute);

                if ((objects == null) || (objects.getItems() == null) || (objects.getItems().isEmpty())) {
                    return false;
                }

                                objects.getItems().forEach(action::accept);

                                                String next = objects.getNextPageToken();
                if (next != null) {
                    list.setPageToken(next);
                    return true;
                }

                return false;
            } catch (Exception e) {
                throw new BlobStoreException("Exception while listing objects", e);
            }
        }

        @Override
        public Spliterator<StorageObject> trySplit() {
            return null;
        }

        @Override
        public long estimateSize() {
            return Long.MAX_VALUE;
        }

        @Override
        public int characteristics() {
            return 0;
        }
    }

    
    static Stream<StorageObject> blobsStream(Storage client, String bucketName, String prefix, long pageSize) throws IOException {
        return StreamSupport.stream(new StorageObjectsSpliterator(client, bucketName, prefix, pageSize), false);
    }

}
