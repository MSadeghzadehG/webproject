
package org.elasticsearch.ingest.geoip;

import com.fasterxml.jackson.databind.JsonNode;
import com.maxmind.db.NodeCache;
import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.cache.Cache;
import org.elasticsearch.common.cache.CacheBuilder;

import java.io.IOException;
import java.util.concurrent.ExecutionException;

final class GeoIpCache implements NodeCache {
    private final Cache<Integer, JsonNode> cache;

    GeoIpCache(long maxSize) {
        this.cache = CacheBuilder.<Integer, JsonNode>builder().setMaximumWeight(maxSize).build();
    }

    @Override
    public JsonNode get(int key, Loader loader) throws IOException {
        try {
            return cache.computeIfAbsent(key, loader::load);
        } catch (ExecutionException e) {
            Throwable cause = e.getCause() != null ? e.getCause() : e;
            throw new ElasticsearchException(cause);
        }
    }
}
