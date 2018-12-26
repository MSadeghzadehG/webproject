

package org.elasticsearch.index.seqno;

import org.elasticsearch.action.admin.indices.stats.IndexShardStats;
import org.elasticsearch.action.admin.indices.stats.IndexStats;
import org.elasticsearch.action.admin.indices.stats.IndicesStatsResponse;
import org.elasticsearch.action.admin.indices.stats.ShardStats;
import org.elasticsearch.client.Client;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.cluster.node.DiscoveryNodes;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.index.IndexService;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.InternalSettingsPlugin;
import org.elasticsearch.test.transport.MockTransportService;
import org.elasticsearch.transport.TransportRequest;
import org.elasticsearch.transport.TransportRequestOptions;
import org.elasticsearch.transport.TransportService;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.function.Consumer;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static org.hamcrest.Matchers.equalTo;

public class GlobalCheckpointSyncIT extends ESIntegTestCase {

    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Stream.concat(
                super.nodePlugins().stream(),
                Stream.of(InternalSettingsPlugin.class, MockTransportService.TestPlugin.class))
                .collect(Collectors.toList());
    }

    public void testPostOperationGlobalCheckpointSync() throws Exception {
                runGlobalCheckpointSyncTest(TimeValue.timeValueHours(24), client -> {}, client -> {});
    }

    
    public void testBackgroundGlobalCheckpointSync() throws Exception {
        runGlobalCheckpointSyncTest(
                TimeValue.timeValueSeconds(randomIntBetween(1, 3)),
                client -> {
                                        final DiscoveryNodes nodes = client.admin().cluster().prepareState().get().getState().getNodes();
                    for (final DiscoveryNode node : nodes) {
                        for (final DiscoveryNode other : nodes) {
                            if (node == other) {
                                continue;
                            }
                            final MockTransportService senderTransportService =
                                    (MockTransportService) internalCluster().getInstance(TransportService.class, node.getName());
                            final MockTransportService receiverTransportService =
                                    (MockTransportService) internalCluster().getInstance(TransportService.class, other.getName());

                            senderTransportService.addDelegate(receiverTransportService,
                                    new MockTransportService.DelegateTransport(senderTransportService.original()) {
                                        @Override
                                        protected void sendRequest(
                                                final Connection connection,
                                                final long requestId,
                                                final String action,
                                                final TransportRequest request,
                                                final TransportRequestOptions options) throws IOException {
                                            if ("indices:admin/seq_no/global_checkpoint_sync[r]".equals(action)) {
                                                throw new IllegalStateException("blocking indices:admin/seq_no/global_checkpoint_sync[r]");
                                            } else {
                                                super.sendRequest(connection, requestId, action, request, options);
                                            }
                                        }
                                    });
                        }
                    }
                },
                client -> {
                                        final DiscoveryNodes nodes = client.admin().cluster().prepareState().get().getState().getNodes();
                    for (final DiscoveryNode node : nodes) {
                        for (final DiscoveryNode other : nodes) {
                            if (node == other) {
                                continue;
                            }
                            final MockTransportService senderTransportService =
                                    (MockTransportService) internalCluster().getInstance(TransportService.class, node.getName());
                            final MockTransportService receiverTransportService =
                                    (MockTransportService) internalCluster().getInstance(TransportService.class, other.getName());
                            senderTransportService.clearRule(receiverTransportService);
                        }
                    }
                });
    }

    private void runGlobalCheckpointSyncTest(
            final TimeValue globalCheckpointSyncInterval,
            final Consumer<Client> beforeIndexing,
            final Consumer<Client> afterIndexing) throws Exception {
        final int numberOfReplicas = randomIntBetween(1, 4);
        internalCluster().ensureAtLeastNumDataNodes(1 + numberOfReplicas);
        prepareCreate(
                "test",
                Settings.builder()
                        .put(IndexService.GLOBAL_CHECKPOINT_SYNC_INTERVAL_SETTING.getKey(), globalCheckpointSyncInterval)
                        .put("index.number_of_replicas", numberOfReplicas))
                .get();
        if (randomBoolean()) {
            ensureGreen();
        }

        beforeIndexing.accept(client());

        final int numberOfDocuments = randomIntBetween(0, 256);

        final int numberOfThreads = randomIntBetween(1, 4);
        final CyclicBarrier barrier = new CyclicBarrier(1 + numberOfThreads);

                final List<Thread> threads = new ArrayList<>(numberOfThreads);
        for (int i = 0; i < numberOfThreads; i++) {
            final int index = i;
            final Thread thread = new Thread(() -> {
                try {
                    barrier.await();
                } catch (BrokenBarrierException | InterruptedException e) {
                    throw new RuntimeException(e);
                }
                for (int j = 0; j < numberOfDocuments; j++) {
                    final String id = Integer.toString(index * numberOfDocuments + j);
                    client().prepareIndex("test", "test", id).setSource("{\"foo\": " + id + "}", XContentType.JSON).get();
                }
                try {
                    barrier.await();
                } catch (BrokenBarrierException | InterruptedException e) {
                    throw new RuntimeException(e);
                }
            });
            threads.add(thread);
            thread.start();
        }

                barrier.await();

                barrier.await();

        afterIndexing.accept(client());

        assertBusy(() -> {
            final IndicesStatsResponse stats = client().admin().indices().prepareStats().clear().get();
            final IndexStats indexStats = stats.getIndex("test");
            for (final IndexShardStats indexShardStats : indexStats.getIndexShards().values()) {
                Optional<ShardStats> maybePrimary =
                        Stream.of(indexShardStats.getShards())
                                .filter(s -> s.getShardRouting().active() && s.getShardRouting().primary())
                                .findFirst();
                if (!maybePrimary.isPresent()) {
                    continue;
                }
                final ShardStats primary = maybePrimary.get();
                final SeqNoStats primarySeqNoStats = primary.getSeqNoStats();
                for (final ShardStats shardStats : indexShardStats) {
                    final SeqNoStats seqNoStats = shardStats.getSeqNoStats();
                    if (seqNoStats == null) {
                                                continue;
                    }
                    assertThat(seqNoStats.getGlobalCheckpoint(), equalTo(primarySeqNoStats.getGlobalCheckpoint()));
                }
            }
        });

        for (final Thread thread : threads) {
            thread.join();
        }
    }

}
