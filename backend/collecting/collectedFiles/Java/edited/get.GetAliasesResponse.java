

package org.elasticsearch.action.admin.indices.alias.get;

import com.carrotsearch.hppc.cursors.ObjectObjectCursor;
import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.cluster.metadata.AliasMetaData;
import org.elasticsearch.common.collect.ImmutableOpenMap;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class GetAliasesResponse extends ActionResponse {

    private ImmutableOpenMap<String, List<AliasMetaData>> aliases = ImmutableOpenMap.of();

    public GetAliasesResponse(ImmutableOpenMap<String, List<AliasMetaData>> aliases) {
        this.aliases = aliases;
    }

    GetAliasesResponse() {
    }


    public ImmutableOpenMap<String, List<AliasMetaData>> getAliases() {
        return aliases;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        ImmutableOpenMap.Builder<String, List<AliasMetaData>> aliasesBuilder = ImmutableOpenMap.builder();
        for (int i = 0; i < size; i++) {
            String key = in.readString();
            int valueSize = in.readVInt();
            List<AliasMetaData> value = new ArrayList<>(valueSize);
            for (int j = 0; j < valueSize; j++) {
                value.add(new AliasMetaData(in));
            }
            aliasesBuilder.put(key, Collections.unmodifiableList(value));
        }
        aliases = aliasesBuilder.build();
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(aliases.size());
        for (ObjectObjectCursor<String, List<AliasMetaData>> entry : aliases) {
            out.writeString(entry.key);
            out.writeVInt(entry.value.size());
            for (AliasMetaData aliasMetaData : entry.value) {
                aliasMetaData.writeTo(out);
            }
        }
    }
}
