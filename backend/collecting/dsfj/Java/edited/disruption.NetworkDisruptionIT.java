

package org.elasticsearch.test.disruption;

import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.InternalTestCluster;
import org.elasticsearch.test.disruption.NetworkDisruption.TwoPartitions;
import org.elasticsearch.test.transport.MockTransportService;
import org.elasticsearch.transport.TransportService;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import static org.hamcrest.Matchers.greaterThanOrEqualTo;

public class NetworkDisruptionIT extends ESIntegTestCase {
    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Arrays.asList(MockTransportService.TestPlugin.class);
    }

    public void testNetworkPartitionWithNodeShutdown() throws IOException {
        internalCluster().ensureAtLeastNumDataNodes(2);
        String[] nodeNames = internalCluster().getNodeNames();
        NetworkDisruption networkDisruption =
                new NetworkDisruption(new TwoPartitions(nodeNames[0], nodeNames[1]), new NetworkDisruption.NetworkUnresponsive());
        internalCluster().setDisruptionScheme(networkDisruption);
        networkDisruption.startDisrupting();
        internalCluster().stopRandomNode(InternalTestCluster.nameFilter(nodeNames[0]));
        internalCluster().clearDisruptionScheme();
    }

    public void testNetworkPartitionRemovalRestoresConnections() throws IOException {
        Set<String> nodes = new HashSet<>();
        nodes.addAll(Arrays.asList(internalCluster().getNodeNames()));
        nodes.remove(internalCluster().getMasterName());
        if (nodes.size() <= 2) {
            internalCluster().ensureAtLeastNumDataNodes(3 - nodes.size());
            nodes.addAll(Arrays.asList(internalCluster().getNodeNames()));
            nodes.remove(internalCluster().getMasterName());
        }
        Set<String> side1 = new HashSet<>(randomSubsetOf(randomIntBetween(1, nodes.size() - 1), nodes));
        Set<String> side2 = new HashSet<>(nodes);
        side2.removeAll(side1);
        assertThat(side2.size(), greaterThanOrEqualTo(1));
        NetworkDisruption networkDisruption = new NetworkDisruption(new TwoPartitions(side1, side2),
            new NetworkDisruption.NetworkDisconnect());
        internalCluster().setDisruptionScheme(networkDisruption);
        networkDisruption.startDisrupting();
                client(randomFrom(side1)).admin().cluster().prepareNodesInfo().get();
        client(randomFrom(side2)).admin().cluster().prepareNodesInfo().get();
        internalCluster().clearDisruptionScheme();
                for (String nodeA : side1) {
            for (String nodeB : side2) {
                TransportService serviceA = internalCluster().getInstance(TransportService.class, nodeA);
                TransportService serviceB = internalCluster().getInstance(TransportService.class, nodeB);
                assertTrue(nodeA + " is not connected to " + nodeB, serviceA.nodeConnected(serviceB.getLocalNode()));
                assertTrue(nodeB + " is not connected to " + nodeA, serviceB.nodeConnected(serviceA.getLocalNode()));
            }
        }
    }

}
