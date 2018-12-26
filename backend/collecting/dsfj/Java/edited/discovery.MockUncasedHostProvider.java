
package org.elasticsearch.test.discovery;

import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.util.concurrent.ConcurrentCollections;
import org.elasticsearch.discovery.zen.UnicastHostsProvider;

import java.io.Closeable;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.function.Supplier;
import java.util.stream.Collectors;


public final class MockUncasedHostProvider implements UnicastHostsProvider, Closeable {

    static final Map<ClusterName, Set<MockUncasedHostProvider>> activeNodesPerCluster = new HashMap<>();


    private final Supplier<DiscoveryNode> localNodeSupplier;
    private final ClusterName clusterName;

    public MockUncasedHostProvider(Supplier<DiscoveryNode> localNodeSupplier, ClusterName clusterName) {
        this.localNodeSupplier = localNodeSupplier;
        this.clusterName = clusterName;
        synchronized (activeNodesPerCluster) {
            getActiveNodesForCurrentCluster().add(this);
        }
    }

    @Override
    public List<DiscoveryNode> buildDynamicNodes() {
        final DiscoveryNode localNode = getNode();
        assert localNode != null;
        synchronized (activeNodesPerCluster) {
            Set<MockUncasedHostProvider> activeNodes = getActiveNodesForCurrentCluster();
            return activeNodes.stream()
                .map(MockUncasedHostProvider::getNode)
                .filter(Objects::nonNull)
                .filter(n -> localNode.equals(n) == false)
                .collect(Collectors.toList());
        }
    }

    @Nullable
    private DiscoveryNode getNode() {
        return localNodeSupplier.get();
    }

    private Set<MockUncasedHostProvider> getActiveNodesForCurrentCluster() {
        assert Thread.holdsLock(activeNodesPerCluster);
        return activeNodesPerCluster.computeIfAbsent(clusterName,
            clusterName -> ConcurrentCollections.newConcurrentSet());
    }

    @Override
    public void close() {
        synchronized (activeNodesPerCluster) {
            boolean found = getActiveNodesForCurrentCluster().remove(this);
            assert found;
        }
    }
}
