

package org.elasticsearch.action.fieldcaps;

import org.elasticsearch.Version;
import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Map;


public class FieldCapabilitiesResponse extends ActionResponse implements ToXContentFragment {
    private Map<String, Map<String, FieldCapabilities>> responseMap;
    private List<FieldCapabilitiesIndexResponse> indexResponses;

    FieldCapabilitiesResponse(Map<String, Map<String, FieldCapabilities>> responseMap) {
        this(responseMap, Collections.emptyList());
    }

    FieldCapabilitiesResponse(List<FieldCapabilitiesIndexResponse> indexResponses) {
        this(Collections.emptyMap(), indexResponses);
    }

    private FieldCapabilitiesResponse(Map<String, Map<String, FieldCapabilities>> responseMap,
                                      List<FieldCapabilitiesIndexResponse> indexResponses) {
        this.responseMap = responseMap;
        this.indexResponses = indexResponses;
    }

    
    FieldCapabilitiesResponse() {
        this.responseMap = Collections.emptyMap();
    }

    
    public Map<String, Map<String, FieldCapabilities>> get() {
        return responseMap;
    }


    
    List<FieldCapabilitiesIndexResponse> getIndexResponses() {
        return indexResponses;
    }
    
    public Map<String, FieldCapabilities> getField(String field) {
        return responseMap.get(field);
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        this.responseMap =
            in.readMap(StreamInput::readString, FieldCapabilitiesResponse::readField);
        if (in.getVersion().onOrAfter(Version.V_5_5_0)) {
            indexResponses = in.readList(FieldCapabilitiesIndexResponse::new);
        } else {
            indexResponses = Collections.emptyList();
        }
    }

    private static Map<String, FieldCapabilities> readField(StreamInput in) throws IOException {
        return in.readMap(StreamInput::readString, FieldCapabilities::new);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeMap(responseMap, StreamOutput::writeString, FieldCapabilitiesResponse::writeField);
        if (out.getVersion().onOrAfter(Version.V_5_5_0)) {
            out.writeList(indexResponses);
        }

    }

    private static void writeField(StreamOutput out,
                           Map<String, FieldCapabilities> map) throws IOException {
        out.writeMap(map, StreamOutput::writeString, (valueOut, fc) -> fc.writeTo(valueOut));
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.field("fields", responseMap);
        return builder;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        FieldCapabilitiesResponse that = (FieldCapabilitiesResponse) o;

        return responseMap.equals(that.responseMap);
    }

    @Override
    public int hashCode() {
        return responseMap.hashCode();
    }
}
