
package org.elasticsearch.action.admin.indices.template.get;

import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.cluster.metadata.IndexTemplateMetaData;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContent;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static java.util.Collections.singletonMap;

public class GetIndexTemplatesResponse extends ActionResponse implements ToXContentObject {

    private List<IndexTemplateMetaData> indexTemplates;

    GetIndexTemplatesResponse() {
    }

    GetIndexTemplatesResponse(List<IndexTemplateMetaData> indexTemplates) {
        this.indexTemplates = indexTemplates;
    }

    public List<IndexTemplateMetaData> getIndexTemplates() {
        return indexTemplates;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        indexTemplates = new ArrayList<>(size);
        for (int i = 0 ; i < size ; i++) {
            indexTemplates.add(0, IndexTemplateMetaData.readFrom(in));
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(indexTemplates.size());
        for (IndexTemplateMetaData indexTemplate : indexTemplates) {
            indexTemplate.writeTo(out);
        }
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        params = new ToXContent.DelegatingMapParams(singletonMap("reduce_mappings", "true"), params);
        builder.startObject();
        for (IndexTemplateMetaData indexTemplateMetaData : getIndexTemplates()) {
            IndexTemplateMetaData.Builder.toXContent(indexTemplateMetaData, builder, params);
        }
        builder.endObject();
        return builder;
    }
}
