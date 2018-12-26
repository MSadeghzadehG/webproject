

package org.elasticsearch.common.xcontent.yaml;

import com.fasterxml.jackson.core.JsonGenerator;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.common.xcontent.json.JsonXContentGenerator;

import java.io.OutputStream;
import java.util.Set;

public class YamlXContentGenerator extends JsonXContentGenerator {

    public YamlXContentGenerator(JsonGenerator jsonGenerator, OutputStream os, Set<String> includes, Set<String> excludes) {
        super(jsonGenerator, os, includes, excludes);
    }

    @Override
    public XContentType contentType() {
        return XContentType.YAML;
    }

    @Override
    public void usePrintLineFeedAtEnd() {
            }

    @Override
    protected boolean supportsRawWrites() {
        return false;
    }
}
