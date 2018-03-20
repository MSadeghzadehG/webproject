

package org.elasticsearch.action.admin.cluster.snapshots.status;

import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class SnapshotsStatusResponse extends ActionResponse implements ToXContentObject {

    private List<SnapshotStatus> snapshots = Collections.emptyList();

    SnapshotsStatusResponse() {
    }

    SnapshotsStatusResponse(List<SnapshotStatus> snapshots) {
        this.snapshots = snapshots;
    }

    
    public List<SnapshotStatus> getSnapshots() {
        return snapshots;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        int size = in.readVInt();
        List<SnapshotStatus> builder = new ArrayList<>();
        for (int i = 0; i < size; i++) {
            builder.add(SnapshotStatus.readSnapshotStatus(in));
        }
        snapshots = Collections.unmodifiableList(builder);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeVInt(snapshots.size());
        for (SnapshotStatus snapshotInfo : snapshots) {
            snapshotInfo.writeTo(out);
        }
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        builder.startArray("snapshots");
        for (SnapshotStatus snapshot : snapshots) {
            snapshot.toXContent(builder, params);
        }
        builder.endArray();
        builder.endObject();
        return builder;
    }

}
