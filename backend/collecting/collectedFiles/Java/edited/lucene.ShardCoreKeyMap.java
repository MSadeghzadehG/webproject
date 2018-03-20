

package org.elasticsearch.common.lucene;

import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.LeafReader;
import org.elasticsearch.Assertions;
import org.elasticsearch.index.shard.ShardId;
import org.elasticsearch.index.shard.ShardUtils;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;


public final class ShardCoreKeyMap {

    private final Map<IndexReader.CacheKey, ShardId> coreKeyToShard;
    private final Map<String, Set<IndexReader.CacheKey>> indexToCoreKey;

    public ShardCoreKeyMap() {
        coreKeyToShard = new ConcurrentHashMap<>();
        indexToCoreKey = new HashMap<>();
    }

    
    public void add(LeafReader reader) {
        final ShardId shardId = ShardUtils.extractShardId(reader);
        if (shardId == null) {
            throw new IllegalArgumentException("Could not extract shard id from " + reader);
        }
        final IndexReader.CacheHelper cacheHelper = reader.getCoreCacheHelper();
        if (cacheHelper == null) {
            throw new IllegalArgumentException("Reader " + reader + " does not support caching");
        }
        final IndexReader.CacheKey coreKey = cacheHelper.getKey();

        if (coreKeyToShard.containsKey(coreKey)) {
                                                return;
        }

        final String index = shardId.getIndexName();
        synchronized (this) {
            if (coreKeyToShard.containsKey(coreKey) == false) {
                Set<IndexReader.CacheKey> objects = indexToCoreKey.get(index);
                if (objects == null) {
                    objects = new HashSet<>();
                    indexToCoreKey.put(index, objects);
                }
                final boolean added = objects.add(coreKey);
                assert added;
                IndexReader.ClosedListener listener = ownerCoreCacheKey -> {
                    assert coreKey == ownerCoreCacheKey;
                    synchronized (ShardCoreKeyMap.this) {
                        coreKeyToShard.remove(ownerCoreCacheKey);
                        final Set<IndexReader.CacheKey> coreKeys = indexToCoreKey.get(index);
                        final boolean removed = coreKeys.remove(coreKey);
                        assert removed;
                        if (coreKeys.isEmpty()) {
                            indexToCoreKey.remove(index);
                        }
                    }
                };
                boolean addedListener = false;
                try {
                    cacheHelper.addClosedListener(listener);
                    addedListener = true;

                                                                                                                        ShardId previous = coreKeyToShard.put(coreKey, shardId);
                    assert previous == null;
                } finally {
                    if (false == addedListener) {
                        try {
                            listener.onClose(coreKey);
                        } catch (IOException e) {
                            throw new RuntimeException("Blow up trying to recover from failure to add listener", e);
                        }
                    }
                }
            }
        }
    }

    
    public synchronized ShardId getShardId(Object coreKey) {
        return coreKeyToShard.get(coreKey);
    }

    
    public synchronized Set<Object> getCoreKeysForIndex(String index) {
        final Set<IndexReader.CacheKey> objects = indexToCoreKey.get(index);
        if (objects == null) {
            return Collections.emptySet();
        }
                return Collections.unmodifiableSet(new HashSet<>(objects));
    }

    
    public synchronized int size() {
        assert assertSize();
        return coreKeyToShard.size();
    }

    private synchronized boolean assertSize() {
        if (!Assertions.ENABLED) {
            throw new AssertionError("only run this if assertions are enabled");
        }
        Collection<Set<IndexReader.CacheKey>> values = indexToCoreKey.values();
        int size = 0;
        for (Set<IndexReader.CacheKey> value : values) {
            size += value.size();
        }
        return size == coreKeyToShard.size();
    }

}
