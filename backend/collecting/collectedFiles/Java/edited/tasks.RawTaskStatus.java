

package org.elasticsearch.tasks;

import org.elasticsearch.common.Strings;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

import static java.util.Objects.requireNonNull;
import static org.elasticsearch.common.xcontent.XContentHelper.convertToMap;


public class RawTaskStatus implements Task.Status {
    public static final String NAME = "raw";

    private final BytesReference status;

    public RawTaskStatus(BytesReference status) {
        this.status = requireNonNull(status, "status may not be null");
    }

    
    public RawTaskStatus(StreamInput in) throws IOException {
        status = in.readOptionalBytesReference();
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeOptionalBytesReference(status);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        try (InputStream stream = status.streamInput()) {
            return builder.rawValue(stream, XContentFactory.xContentType(status));
        }
    }

    @Override
    public String getWriteableName() {
        return NAME;
    }

    @Override
    public String toString() {
        return Strings.toString(this);
    }

    
    public Map<String, Object> toMap() {
        return convertToMap(status, false).v2();
    }

        @Override
    public boolean equals(Object obj) {
        if (obj == null || obj.getClass() != RawTaskStatus.class) {
            return false;
        }
        RawTaskStatus other = (RawTaskStatus) obj;
                return toMap().equals(other.toMap());
    }

    @Override
    public int hashCode() {
                return toMap().hashCode();
    }
}
