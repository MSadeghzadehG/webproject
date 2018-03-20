
package org.elasticsearch.indices.flush;

import org.elasticsearch.ExceptionsHelper;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.index.engine.Engine;
import org.elasticsearch.index.shard.ShardId;
import org.elasticsearch.test.InternalTestCluster;

import java.util.List;
import java.util.Map;
import java.util.concurrent.CountDownLatch;


public class SyncedFlushUtil {

    private SyncedFlushUtil() {

    }

    
    public static ShardsSyncedFlushResult attemptSyncedFlush(InternalTestCluster cluster, ShardId shardId) {
        SyncedFlushService service = cluster.getInstance(SyncedFlushService.class);
        LatchedListener<ShardsSyncedFlushResult> listener = new LatchedListener<>();
        service.attemptSyncedFlush(shardId, listener);
        try {
            listener.latch.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        if (listener.error != null) {
            throw ExceptionsHelper.convertToElastic(listener.error);
        }
        return listener.result;
    }

    public static final class LatchedListener<T> implements ActionListener<T> {
        public volatile T result;
        public volatile Exception error;
        public final CountDownLatch latch = new CountDownLatch(1);

        @Override
        public void onResponse(T result) {
            this.result = result;
            latch.countDown();
        }

        @Override
        public void onFailure(Exception e) {
            error = e;
            latch.countDown();
        }
    }

    
    public static Map<String, SyncedFlushService.PreSyncedFlushResponse> sendPreSyncRequests(SyncedFlushService service, List<ShardRouting> activeShards, ClusterState state, ShardId shardId) {
        LatchedListener<Map<String, SyncedFlushService.PreSyncedFlushResponse>> listener = new LatchedListener<>();
        service.sendPreSyncRequests(activeShards, state, shardId, listener);
        try {
            listener.latch.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        if (listener.error != null) {
            throw ExceptionsHelper.convertToElastic(listener.error);
        }
        return listener.result;
    }
}
