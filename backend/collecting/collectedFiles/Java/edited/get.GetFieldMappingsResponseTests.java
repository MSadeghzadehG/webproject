

package org.elasticsearch.action.admin.indices.mapping.get;

import org.elasticsearch.action.admin.indices.mapping.get.GetFieldMappingsResponse.FieldMappingMetaData;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.common.io.stream.BytesStreamOutput;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class GetFieldMappingsResponseTests extends ESTestCase {

    public void testSerialization() throws IOException {
        Map<String, Map<String, Map<String, FieldMappingMetaData>>> mappings = new HashMap<>();
        FieldMappingMetaData fieldMappingMetaData = new FieldMappingMetaData("my field", new BytesArray("{}"));
        mappings.put("index", Collections.singletonMap("type", Collections.singletonMap("field", fieldMappingMetaData)));
        GetFieldMappingsResponse response = new GetFieldMappingsResponse(mappings);

        try (BytesStreamOutput out = new BytesStreamOutput()) {
            response.writeTo(out);
            GetFieldMappingsResponse serialized = new GetFieldMappingsResponse();
            try (StreamInput in = StreamInput.wrap(out.bytes().toBytesRef().bytes)) {
                serialized.readFrom(in);
                FieldMappingMetaData metaData = serialized.fieldMappings("index", "type", "field");
                assertNotNull(metaData);
                assertEquals(new BytesArray("{}"), metaData.getSource());
            }
        }
    }
}
