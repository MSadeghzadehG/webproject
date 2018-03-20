

package org.elasticsearch.cluster.routing;

import org.elasticsearch.common.util.CollectionUtils;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;


public class RotationShardShuffler extends ShardShuffler {

    private final AtomicInteger seed;

    public RotationShardShuffler(int seed) {
        this.seed = new AtomicInteger(seed);
    }

    @Override
    public int nextSeed() {
        return seed.getAndIncrement();
    }

    @Override
    public List<ShardRouting> shuffle(List<ShardRouting> shards, int seed) {
        return CollectionUtils.rotate(shards, seed);
    }

}
