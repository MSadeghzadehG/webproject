
package org.elasticsearch.benchmark.routing.allocation;

import org.elasticsearch.Version;
import org.elasticsearch.cluster.ClusterModule;
import org.elasticsearch.cluster.EmptyClusterInfoService;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.cluster.routing.allocation.AllocationService;
import org.elasticsearch.cluster.routing.allocation.FailedShard;
import org.elasticsearch.cluster.routing.allocation.RoutingAllocation;
import org.elasticsearch.cluster.routing.allocation.allocator.BalancedShardsAllocator;
import org.elasticsearch.cluster.routing.allocation.decider.AllocationDecider;
import org.elasticsearch.cluster.routing.allocation.decider.AllocationDeciders;
import org.elasticsearch.common.settings.ClusterSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.util.set.Sets;
import org.elasticsearch.gateway.GatewayAllocator;

import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

public final class Allocators {
    private static class NoopGatewayAllocator extends GatewayAllocator {
        public static final NoopGatewayAllocator INSTANCE = new NoopGatewayAllocator();

        protected NoopGatewayAllocator() {
            super(Settings.EMPTY);
        }

        @Override
        public void applyStartedShards(RoutingAllocation allocation, List<ShardRouting> startedShards) {
                    }

        @Override
        public void applyFailedShards(RoutingAllocation allocation, List<FailedShard> failedShards) {
                    }

        @Override
        public void allocateUnassigned(RoutingAllocation allocation) {
                    }
    }

    private Allocators() {
        throw new AssertionError("Do not instantiate");
    }


    public static AllocationService createAllocationService(Settings settings) throws NoSuchMethodException, InstantiationException,
        IllegalAccessException, InvocationTargetException {
        return createAllocationService(settings, new ClusterSettings(Settings.EMPTY, ClusterSettings
            .BUILT_IN_CLUSTER_SETTINGS));
    }

    public static AllocationService createAllocationService(Settings settings, ClusterSettings clusterSettings) throws
        InvocationTargetException, NoSuchMethodException, InstantiationException, IllegalAccessException {
        return new AllocationService(settings,
            defaultAllocationDeciders(settings, clusterSettings),
            NoopGatewayAllocator.INSTANCE, new BalancedShardsAllocator(settings), EmptyClusterInfoService.INSTANCE);
    }

    public static AllocationDeciders defaultAllocationDeciders(Settings settings, ClusterSettings clusterSettings) throws
        IllegalAccessException, InvocationTargetException, InstantiationException, NoSuchMethodException {
        Collection<AllocationDecider> deciders =
            ClusterModule.createAllocationDeciders(settings, clusterSettings, Collections.emptyList());
        return new AllocationDeciders(settings, deciders);

    }

    private static final AtomicInteger portGenerator = new AtomicInteger();

    public static DiscoveryNode newNode(String nodeId, Map<String, String> attributes) {
        return new DiscoveryNode("", nodeId, new TransportAddress(TransportAddress.META_ADDRESS,
            portGenerator.incrementAndGet()), attributes, Sets.newHashSet(DiscoveryNode.Role.MASTER,
            DiscoveryNode.Role.DATA), Version.CURRENT);
    }
}
