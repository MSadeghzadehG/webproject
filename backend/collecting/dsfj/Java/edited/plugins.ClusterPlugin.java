

package org.elasticsearch.plugins;

import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.function.Supplier;

import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.routing.allocation.allocator.ShardsAllocator;
import org.elasticsearch.cluster.routing.allocation.decider.AllocationDecider;
import org.elasticsearch.common.settings.ClusterSettings;
import org.elasticsearch.common.settings.Settings;


public interface ClusterPlugin {

    
    default Collection<AllocationDecider> createAllocationDeciders(Settings settings, ClusterSettings clusterSettings) {
        return Collections.emptyList();
    }

    
    default Map<String, Supplier<ShardsAllocator>> getShardsAllocators(Settings settings, ClusterSettings clusterSettings) {
        return Collections.emptyMap();
    }

    
    default void onNodeStarted() {
    }

    
    default Map<String, Supplier<ClusterState.Custom>> getInitialClusterStateCustomSupplier() { return Collections.emptyMap(); }
}
