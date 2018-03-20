

package org.elasticsearch.common.xcontent.smile;

import com.fasterxml.jackson.core.JsonGenerator;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.common.xcontent.json.JsonXContentGenerator;

import java.io.OutputStream;
import java.util.Set;

public class SmileXContentGenerator extends JsonXContentGenerator {

    public SmileXContentGenerator(JsonGenerator jsonGenerator, OutputStream os, Set<String> includes, Set<String> excludes) {
        super(jsonGenerator, os, includes, excludes);
    }

    @Override
    public XContentType contentType() {
        return XContentType.SMILE;
    }

    @Override
    public void usePrintLineFeedAtEnd() {
            }

    @Override
    protected boolean supportsRawWrites() {
        return false;
    }
}
