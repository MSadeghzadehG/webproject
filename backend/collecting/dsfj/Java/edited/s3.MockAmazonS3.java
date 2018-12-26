

package org.elasticsearch.repositories.s3;

import com.amazonaws.AmazonClientException;
import com.amazonaws.AmazonServiceException;
import com.amazonaws.SdkClientException;
import com.amazonaws.services.s3.AbstractAmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CopyObjectRequest;
import com.amazonaws.services.s3.model.CopyObjectResult;
import com.amazonaws.services.s3.model.DeleteObjectRequest;
import com.amazonaws.services.s3.model.GetObjectMetadataRequest;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ListObjectsRequest;
import com.amazonaws.services.s3.model.ObjectListing;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.amazonaws.services.s3.model.S3ObjectSummary;

import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.net.InetAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import static org.junit.Assert.assertTrue;

class MockAmazonS3 extends AbstractAmazonS3 {

    private final int mockSocketPort;

    private Map<String, InputStream> blobs = new ConcurrentHashMap<>();

            private byte[] byteCounter = new byte[100];


    MockAmazonS3(int mockSocketPort) {
        this.mockSocketPort = mockSocketPort;
    }

                                    private void simulateS3SocketConnection() {
        try (Socket socket = new Socket(InetAddress.getByName("127.0.0.1"), mockSocketPort)) {
            assertTrue(socket.isConnected());         } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }


    @Override
    public boolean doesBucketExist(String bucket) {
        return true;
    }

    @Override
    public boolean doesObjectExist(String bucketName, String objectName) throws AmazonServiceException, SdkClientException {
        simulateS3SocketConnection();
        return blobs.containsKey(objectName);
    }

    @Override
    public ObjectMetadata getObjectMetadata(
            GetObjectMetadataRequest getObjectMetadataRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
        String blobName = getObjectMetadataRequest.getKey();

        if (!blobs.containsKey(blobName)) {
            throw new AmazonS3Exception("[" + blobName + "] does not exist.");
        }

        return new ObjectMetadata();     }

    @Override
    public PutObjectResult putObject(PutObjectRequest putObjectRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
        String blobName = putObjectRequest.getKey();

        if (blobs.containsKey(blobName)) {
            throw new AmazonS3Exception("[" + blobName + "] already exists.");
        }

        blobs.put(blobName, putObjectRequest.getInputStream());
        return new PutObjectResult();
    }

    @Override
    public S3Object getObject(GetObjectRequest getObjectRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
                        String blobName = getObjectRequest.getKey();

        if (!blobs.containsKey(blobName)) {
            throw new AmazonS3Exception("[" + blobName + "] does not exist.");
        }

                S3ObjectInputStream stream = new S3ObjectInputStream(
                blobs.get(blobName), null, false);
        S3Object s3Object = new S3Object();
        s3Object.setObjectContent(stream);
        return s3Object;
    }

    @Override
    public ObjectListing listObjects(ListObjectsRequest listObjectsRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
        MockObjectListing list = new MockObjectListing();
        list.setTruncated(false);

        String blobName;
        String prefix = listObjectsRequest.getPrefix();

        ArrayList<S3ObjectSummary> mockObjectSummaries = new ArrayList<>();

        for (Map.Entry<String, InputStream> blob : blobs.entrySet()) {
            blobName = blob.getKey();
            S3ObjectSummary objectSummary = new S3ObjectSummary();

            if (prefix.isEmpty() || blobName.startsWith(prefix)) {
                objectSummary.setKey(blobName);

                try {
                    objectSummary.setSize(getSize(blob.getValue()));
                } catch (IOException e) {
                    throw new AmazonS3Exception("Object listing " +
                            "failed for blob [" + blob.getKey() + "]");
                }

                mockObjectSummaries.add(objectSummary);
            }
        }

        list.setObjectSummaries(mockObjectSummaries);
        return list;
    }

    @Override
    public CopyObjectResult copyObject(CopyObjectRequest copyObjectRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
        String sourceBlobName = copyObjectRequest.getSourceKey();
        String targetBlobName = copyObjectRequest.getDestinationKey();

        if (!blobs.containsKey(sourceBlobName)) {
            throw new AmazonS3Exception("Source blob [" +
                    sourceBlobName + "] does not exist.");
        }

        if (blobs.containsKey(targetBlobName)) {
            throw new AmazonS3Exception("Target blob [" +
                    targetBlobName + "] already exists.");
        }

        blobs.put(targetBlobName, blobs.get(sourceBlobName));
        return new CopyObjectResult();     }

    @Override
    public void deleteObject(DeleteObjectRequest deleteObjectRequest)
            throws AmazonClientException, AmazonServiceException {
        simulateS3SocketConnection();
        String blobName = deleteObjectRequest.getKey();

        if (!blobs.containsKey(blobName)) {
            throw new AmazonS3Exception("[" + blobName + "] does not exist.");
        }

        blobs.remove(blobName);
    }

    private int getSize(InputStream stream) throws IOException {
        int size = stream.read(byteCounter);
        stream.reset();         return size;
    }

    private class MockObjectListing extends ObjectListing {
                                private List<S3ObjectSummary> mockObjectSummaries;

        @Override
        public List<S3ObjectSummary> getObjectSummaries() {
            return mockObjectSummaries;
        }

        private void setObjectSummaries(List<S3ObjectSummary> objectSummaries) {
            mockObjectSummaries = objectSummaries;
        }
    }
}
