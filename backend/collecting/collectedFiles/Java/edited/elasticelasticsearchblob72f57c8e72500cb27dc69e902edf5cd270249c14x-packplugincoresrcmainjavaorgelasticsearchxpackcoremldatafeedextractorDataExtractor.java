
package org.elasticsearch.xpack.core.ml.datafeed.extractor;

import java.io.IOException;
import java.io.InputStream;
import java.util.Optional;

public interface DataExtractor {

    
    boolean hasNext();

    
    Optional<InputStream> next() throws IOException;

    
    boolean isCancelled();

    
    void cancel();
}
