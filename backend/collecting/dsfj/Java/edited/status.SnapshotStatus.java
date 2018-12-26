

package org.elasticsearch.action.admin.cluster.snapshots.status;

import org.elasticsearch.Version;
import org.elasticsearch.cluster.SnapshotsInProgress.State;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.inject.internal.Nullable;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Streamable;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.snapshots.Snapshot;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import static java.util.Collections.unmodifiableMap;


public class SnapshotStatus implements ToXContentObject, Streamable {

    private Snapshot snapshot;

    private State state;

    private List<SnapshotIndexShardStatus> shards;

    private Map<String, SnapshotIndexStatus> indicesStatus;

    private SnapshotShardsStats shardsStats;

    private SnapshotStats stats;

    @Nullable
    private Boolean includeGlobalState;

    SnapshotStatus(final Snapshot snapshot, final State state, final List<SnapshotIndexShardStatus> shards,
                   final Boolean includeGlobalState) {
        this.snapshot = Objects.requireNonNull(snapshot);
        this.state = Objects.requireNonNull(state);
        this.shards = Objects.requireNonNull(shards);
        this.includeGlobalState = includeGlobalState;
        shardsStats = new SnapshotShardsStats(shards);
        updateShardStats();
    }

    SnapshotStatus() {
    }

    
    public Snapshot getSnapshot() {
        return snapshot;
    }

    
    public State getState() {
        return state;
    }

    
    public Boolean includeGlobalState() {
        return includeGlobalState;
    }

    
    public List<SnapshotIndexShardStatus> getShards() {
        return shards;
    }

    public SnapshotShardsStats getShardsStats() {
        return shardsStats;
    }

    
    public Map<String, SnapshotIndexStatus> getIndices() {
        if (this.indicesStatus != null) {
            return this.indicesStatus;
        }

        Map<String, SnapshotIndexStatus> indicesStatus = new HashMap<>();

        Set<String> indices = new HashSet<>();
        for (SnapshotIndexShardStatus shard : shards) {
            indices.add(shard.getIndex());
        }

        for (String index : indices) {
            List<SnapshotIndexShardStatus> shards = new ArrayList<>();
            for (SnapshotIndexShardStatus shard : this.shards) {
                if (shard.getIndex().equals(index)) {
                    shards.add(shard);
                }
            }
            indicesStatus.put(index, new SnapshotIndexStatus(index, shards));
        }
        this.indicesStatus = unmodifiableMap(indicesStatus);
        return this.indicesStatus;

    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        snapshot = new Snapshot(in);
        state = State.fromValue(in.readByte());
        int size = in.readVInt();
        List<SnapshotIndexShardStatus> builder = new ArrayList<>();
        for (int i = 0; i < size; i++) {
            builder.add(SnapshotIndexShardStatus.readShardSnapshotStatus(in));
        }
        shards = Collections.unmodifiableList(builder);
        if (in.getVersion().onOrAfter(Version.V_6_2_0)) {
            includeGlobalState = in.readOptionalBoolean();
        }
        updateShardStats();
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        snapshot.writeTo(out);
        out.writeByte(state.value());
        out.writeVInt(shards.size());
        for (SnapshotIndexShardStatus shard : shards) {
            shard.writeTo(out);
        }
        if (out.getVersion().onOrAfter(Version.V_6_2_0)) {
            out.writeOptionalBoolean(includeGlobalState);
        }
    }

    
    public static SnapshotStatus readSnapshotStatus(StreamInput in) throws IOException {
        SnapshotStatus snapshotInfo = new SnapshotStatus();
        snapshotInfo.readFrom(in);
        return snapshotInfo;
    }

    @Override
    public String toString() {
        return Strings.toString(this, true, false);
    }

    
    public SnapshotStats getStats() {
        return stats;
    }

    private static final String SNAPSHOT = "snapshot";
    private static final String REPOSITORY = "repository";
    private static final String UUID = "uuid";
    private static final String STATE = "state";
    private static final String INDICES = "indices";
    private static final String INCLUDE_GLOBAL_STATE = "include_global_state";

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        builder.field(SNAPSHOT, snapshot.getSnapshotId().getName());
        builder.field(REPOSITORY, snapshot.getRepository());
        builder.field(UUID, snapshot.getSnapshotId().getUUID());
        builder.field(STATE, state.name());
        if (includeGlobalState != null) {
            builder.field(INCLUDE_GLOBAL_STATE, includeGlobalState);
        }
        shardsStats.toXContent(builder, params);
        stats.toXContent(builder, params);
        builder.startObject(INDICES);
        for (SnapshotIndexStatus indexStatus : getIndices().values()) {
            indexStatus.toXContent(builder, params);
        }
        builder.endObject();
        builder.endObject();
        return builder;
    }

    private void updateShardStats() {
        stats = new SnapshotStats();
        shardsStats = new SnapshotShardsStats(shards);
        for (SnapshotIndexShardStatus shard : shards) {
            stats.add(shard.getStats());
        }
    }
}
