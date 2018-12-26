

package org.elasticsearch.action.admin.indices.segments;

import org.elasticsearch.index.shard.ShardId;

import java.util.Arrays;
import java.util.Iterator;

public class IndexShardSegments implements Iterable<ShardSegments> {

    private final ShardId shardId;

    private final ShardSegments[] shards;

    IndexShardSegments(ShardId shardId, ShardSegments[] shards) {
        this.shardId = shardId;
        this.shards = shards;
    }

    public ShardId getShardId() {
        return this.shardId;
    }

    public ShardSegments getAt(int i) {
        return shards[i];
    }

    public ShardSegments[] getShards() {
        return this.shards;
    }

    @Override
    public Iterator<ShardSegments> iterator() {
        return Arrays.stream(shards).iterator();
    }
}