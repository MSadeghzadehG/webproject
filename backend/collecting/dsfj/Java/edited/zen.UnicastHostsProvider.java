

package org.elasticsearch.discovery.zen;

import org.elasticsearch.cluster.node.DiscoveryNode;

import java.util.List;


public interface UnicastHostsProvider {

    
    List<DiscoveryNode> buildDynamicNodes();
}
