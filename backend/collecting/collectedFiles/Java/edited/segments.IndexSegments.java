

package org.elasticsearch.action.admin.indices.segments;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class IndexSegments implements Iterable<IndexShardSegments> {

    private final String index;

    private final Map<Integer, IndexShardSegments> indexShards;

    IndexSegments(String index, ShardSegments[] shards) {
        this.index = index;

        Map<Integer, List<ShardSegments>> tmpIndexShards = new HashMap<>();
        for (ShardSegments shard : shards) {
            List<ShardSegments> lst = tmpIndexShards.get(shard.getShardRouting().id());
            if (lst == null) {
                lst = new ArrayList<>();
                tmpIndexShards.put(shard.getShardRouting().id(), lst);
            }
            lst.add(shard);
        }
        indexShards = new HashMap<>();
        for (Map.Entry<Integer, List<ShardSegments>> entry : tmpIndexShards.entrySet()) {
            indexShards.put(entry.getKey(), new IndexShardSegments(entry.getValue().get(0).getShardRouting().shardId(), entry.getValue().toArray(new ShardSegments[entry.getValue().size()])));
        }
    }

    public String getIndex() {
        return this.index;
    }

    
    public Map<Integer, IndexShardSegments> getShards() {
        return this.indexShards;
    }

    @Override
    public Iterator<IndexShardSegments> iterator() {
        return indexShards.values().iterator();
    }
}