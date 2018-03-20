

package org.elasticsearch.plugins;

import java.util.Collections;
import java.util.Map;

import org.elasticsearch.ingest.Processor;


public interface IngestPlugin {

    
    default Map<String, Processor.Factory> getProcessors(Processor.Parameters parameters) {
        return Collections.emptyMap();
    }
}
