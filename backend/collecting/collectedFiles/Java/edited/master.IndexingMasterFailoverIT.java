package org.elasticsearch.action.support.master;



import org.elasticsearch.action.DocWriteResponse;
import org.elasticsearch.action.index.IndexResponse;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.discovery.DiscoverySettings;
import org.elasticsearch.discovery.zen.ElectMasterService;
import org.elasticsearch.discovery.zen.FaultDetection;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.discovery.TestZenDiscovery;
import org.elasticsearch.test.disruption.NetworkDisruption;
import org.elasticsearch.test.disruption.NetworkDisruption.NetworkDisconnect;
import org.elasticsearch.test.disruption.NetworkDisruption.TwoPartitions;
import org.elasticsearch.test.junit.annotations.TestLogging;
import org.elasticsearch.test.transport.MockTransportService;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import static org.hamcrest.Matchers.equalTo;

@ESIntegTestCase.ClusterScope(scope = ESIntegTestCase.Scope.TEST, numDataNodes = 0, autoMinMasterNodes = false)
public class IndexingMasterFailoverIT extends ESIntegTestCase {

    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        final HashSet<Class<? extends Plugin>> classes = new HashSet<>(super.nodePlugins());
        classes.add(MockTransportService.TestPlugin.class);
        return classes;
    }

    @Override
    protected Settings nodeSettings(int nodeOrdinal) {
        return Settings.builder().put(super.nodeSettings(nodeOrdinal))
            .put(TestZenDiscovery.USE_MOCK_PINGS.getKey(), false).build();
    }

    
    @TestLogging("_root:DEBUG")
    public void testMasterFailoverDuringIndexingWithMappingChanges() throws Throwable {
        logger.info("--> start 4 nodes, 3 master, 1 data");

        final Settings sharedSettings = Settings.builder()
                .put(FaultDetection.PING_TIMEOUT_SETTING.getKey(), "1s")                 .put(FaultDetection.PING_RETRIES_SETTING.getKey(), "1")                 .put("discovery.zen.join_timeout", "10s")                  .put(DiscoverySettings.PUBLISH_TIMEOUT_SETTING.getKey(), "1s")                 .put(ElectMasterService.DISCOVERY_ZEN_MINIMUM_MASTER_NODES_SETTING.getKey(), 2)
                .build();

        internalCluster().startMasterOnlyNodes(3, sharedSettings);

        String dataNode = internalCluster().startDataOnlyNode(sharedSettings);

        logger.info("--> wait for all nodes to join the cluster");
        ensureStableCluster(4);

                client().admin().indices().prepareCreate("myindex")
                .setSettings(Settings.builder().put("index.number_of_shards", 1).put("index.number_of_replicas", 0))
                .get();
        ensureGreen("myindex");

        final CyclicBarrier barrier = new CyclicBarrier(2);

        Thread indexingThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    barrier.await();
                } catch (InterruptedException e) {
                    logger.warn("Barrier interrupted", e);
                    return;
                } catch (BrokenBarrierException e) {
                    logger.warn("Broken barrier", e);
                    return;
                }
                for (int i = 0; i < 10; i++) {
                                        IndexResponse response = client(dataNode).prepareIndex("myindex", "mytype").setSource("field_" + i, "val").get();
                    assertEquals(DocWriteResponse.Result.CREATED, response.getResult());
                }
            }
        });
        indexingThread.setName("indexingThread");
        indexingThread.start();

        barrier.await();

                String master = internalCluster().getMasterName();
        Set<String> otherNodes = new HashSet<>(Arrays.asList(internalCluster().getNodeNames()));
        otherNodes.remove(master);

        NetworkDisruption partition = new NetworkDisruption(
            new TwoPartitions(Collections.singleton(master), otherNodes),
            new NetworkDisconnect());
        internalCluster().setDisruptionScheme(partition);

        logger.info("--> disrupting network");
        partition.startDisrupting();

        logger.info("--> waiting for new master to be elected");
        ensureStableCluster(3, dataNode);

        partition.stopDisrupting();
        logger.info("--> waiting to heal");
        ensureStableCluster(4);

        indexingThread.join();

        ensureGreen("myindex");
        refresh();
        assertThat(client().prepareSearch("myindex").get().getHits().getTotalHits(), equalTo(10L));
    }
}
