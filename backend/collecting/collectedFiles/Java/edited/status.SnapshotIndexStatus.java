

package org.elasticsearch.action.admin.cluster.snapshots.status;

import org.elasticsearch.common.xcontent.ToXContent.Params;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import static java.util.Collections.unmodifiableMap;


public class SnapshotIndexStatus implements Iterable<SnapshotIndexShardStatus>, ToXContentFragment {

    private final String index;

    private final Map<Integer, SnapshotIndexShardStatus> indexShards;

    private final SnapshotShardsStats shardsStats;

    private final SnapshotStats stats;

    SnapshotIndexStatus(String index, Collection<SnapshotIndexShardStatus> shards) {
        this.index = index;

        Map<Integer, SnapshotIndexShardStatus> indexShards = new HashMap<>();
        stats = new SnapshotStats();
        for (SnapshotIndexShardStatus shard : shards) {
            indexShards.put(shard.getShardId().getId(), shard);
            stats.add(shard.getStats());
        }
        shardsStats = new SnapshotShardsStats(shards);
        this.indexShards = unmodifiableMap(indexShards);
    }

    
    public String getIndex() {
        return this.index;
    }

    
    public Map<Integer, SnapshotIndexShardStatus> getShards() {
        return this.indexShards;
    }

    
    public SnapshotShardsStats getShardsStats() {
        return shardsStats;
    }

    
    public SnapshotStats getStats() {
        return stats;
    }

    @Override
    public Iterator<SnapshotIndexShardStatus> iterator() {
        return indexShards.values().iterator();
    }

    static final class Fields {
        static final String SHARDS = "shards";
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject(getIndex());
        shardsStats.toXContent(builder, params);
        stats.toXContent(builder, params);
        builder.startObject(Fields.SHARDS);
        for (SnapshotIndexShardStatus shard : indexShards.values()) {
            shard.toXContent(builder, params);
        }
        builder.endObject();
        builder.endObject();
        return builder;
    }
}
