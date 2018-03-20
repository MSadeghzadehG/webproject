
package org.elasticsearch.common.blobstore;

import java.io.Closeable;
import java.io.IOException;


public interface BlobStore extends Closeable {

    
    BlobContainer blobContainer(BlobPath path);

    
    void delete(BlobPath path) throws IOException;

}
