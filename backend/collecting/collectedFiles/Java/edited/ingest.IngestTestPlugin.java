

package org.elasticsearch.ingest;

import java.util.Collections;
import java.util.Map;

import org.elasticsearch.plugins.IngestPlugin;
import org.elasticsearch.plugins.Plugin;


public class IngestTestPlugin extends Plugin implements IngestPlugin {
    @Override
    public Map<String, Processor.Factory> getProcessors(Processor.Parameters parameters) {
        return Collections.singletonMap("test", (factories, tag, config) ->
            new TestProcessor("id", "test", doc -> {
                doc.setFieldValue("processed", true);
                if (doc.hasField("fail") && doc.getFieldValue("fail", Boolean.class)) {
                    throw new IllegalArgumentException("test processor failed");
                }
            }));
    }
}
