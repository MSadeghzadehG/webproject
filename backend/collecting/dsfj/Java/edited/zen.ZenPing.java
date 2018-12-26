

package org.elasticsearch.discovery.zen;

import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.common.lease.Releasable;
import org.elasticsearch.common.unit.TimeValue;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;
import java.util.function.Consumer;

import static org.elasticsearch.gateway.GatewayService.STATE_NOT_RECOVERED_BLOCK;

public interface ZenPing extends Releasable {

    void start();

    void ping(Consumer<PingCollection> resultsConsumer, TimeValue timeout);

    class PingResponse implements Writeable {

        private static final AtomicLong idGenerator = new AtomicLong();

                        private final long id;

        private final ClusterName clusterName;

        private final DiscoveryNode node;

        private final DiscoveryNode master;

        private final long clusterStateVersion;

        
        PingResponse(DiscoveryNode node, DiscoveryNode master, ClusterName clusterName, long clusterStateVersion) {
            this.id = idGenerator.incrementAndGet();
            this.node = node;
            this.master = master;
            this.clusterName = clusterName;
            this.clusterStateVersion = clusterStateVersion;
        }

        public PingResponse(DiscoveryNode node, DiscoveryNode master, ClusterState state) {
            this(node, master, state.getClusterName(),
                state.blocks().hasGlobalBlock(STATE_NOT_RECOVERED_BLOCK) ?
                    ElectMasterService.MasterCandidate.UNRECOVERED_CLUSTER_VERSION : state.version());
        }

        PingResponse(StreamInput in) throws IOException {
            this.clusterName = new ClusterName(in);
            this.node = new DiscoveryNode(in);
            this.master = in.readOptionalWriteable(DiscoveryNode::new);
            this.clusterStateVersion = in.readLong();
            this.id = in.readLong();
        }

        @Override
        public void writeTo(StreamOutput out) throws IOException {
            clusterName.writeTo(out);
            node.writeTo(out);
            out.writeOptionalWriteable(master);
            out.writeLong(clusterStateVersion);
            out.writeLong(id);
        }

        
        public long id() {
            return this.id;
        }

        
        public ClusterName clusterName() {
            return this.clusterName;
        }

        
        public DiscoveryNode node() {
            return node;
        }

        
        public DiscoveryNode master() {
            return master;
        }

        
        public long getClusterStateVersion() {
            return clusterStateVersion;
        }

        @Override
        public String toString() {
            return "ping_response{node [" + node + "], id[" + id + "], master [" + master + "]," +
                   "cluster_state_version [" + clusterStateVersion + "], cluster_name[" + clusterName.value() + "]}";
        }
    }


    
    class PingCollection {

        Map<DiscoveryNode, PingResponse> pings;

        public PingCollection() {
            pings = new HashMap<>();
        }

        
        public synchronized boolean addPing(PingResponse ping) {
            PingResponse existingResponse = pings.get(ping.node());
                                    if (existingResponse == null || existingResponse.id() <= ping.id()) {
                pings.put(ping.node(), ping);
                return true;
            }
            return false;
        }

        
        public synchronized List<PingResponse> toList() {
            return new ArrayList<>(pings.values());
        }

        
        public synchronized int size() {
            return pings.size();
        }
    }
}
