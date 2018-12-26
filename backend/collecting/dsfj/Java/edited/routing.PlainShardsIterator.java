
package org.elasticsearch.cluster.routing;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public class PlainShardsIterator implements ShardsIterator {

    private final List<ShardRouting> shards;

                private volatile int index;

    public PlainShardsIterator(List<ShardRouting> shards) {
        this.shards = shards;
        reset();
    }

    @Override
    public void reset() {
        index = 0;
    }

    @Override
    public int remaining() {
        return shards.size() - index;
    }

    @Override
    public ShardRouting nextOrNull() {
        if (index == shards.size()) {
            return null;
        } else {
            return shards.get(index++);
        }
    }

    @Override
    public int size() {
        return shards.size();
    }

    @Override
    public int sizeActive() {
        int count = 0;
        for (ShardRouting shard : shards) {
            if (shard.active()) {
                count++;
            }
        }
        return count;
    }

    @Override
    public List<ShardRouting> getShardRoutings() {
        return Collections.unmodifiableList(shards);
    }

    @Override
    public Iterator<ShardRouting> iterator() {
        return shards.iterator();
    }
}
