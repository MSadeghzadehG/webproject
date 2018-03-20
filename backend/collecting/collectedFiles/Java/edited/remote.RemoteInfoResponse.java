

package org.elasticsearch.action.admin.cluster.remote;

import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.transport.RemoteConnectionInfo;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

public final class RemoteInfoResponse extends ActionResponse implements ToXContentObject {

    private List<RemoteConnectionInfo> infos;

    RemoteInfoResponse() {
    }

    RemoteInfoResponse(Collection<RemoteConnectionInfo> infos) {
        this.infos = Collections.unmodifiableList(new ArrayList<>(infos));
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeList(infos);
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        infos = in.readList(RemoteConnectionInfo::new);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        for (RemoteConnectionInfo info : infos) {
            info.toXContent(builder, params);
        }
        builder.endObject();
        return builder;
    }
}
