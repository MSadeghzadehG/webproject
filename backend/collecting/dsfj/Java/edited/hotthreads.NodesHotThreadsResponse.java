

package org.elasticsearch.action.admin.cluster.node.hotthreads;

import org.elasticsearch.action.FailedNodeException;
import org.elasticsearch.action.support.nodes.BaseNodesResponse;
import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.List;

public class NodesHotThreadsResponse extends BaseNodesResponse<NodeHotThreads> {

    NodesHotThreadsResponse() {
    }

    public NodesHotThreadsResponse(ClusterName clusterName, List<NodeHotThreads> nodes, List<FailedNodeException> failures) {
        super(clusterName, nodes, failures);
    }

    @Override
    protected List<NodeHotThreads> readNodesFrom(StreamInput in) throws IOException {
        return in.readList(NodeHotThreads::readNodeHotThreads);
    }

    @Override
    protected void writeNodesTo(StreamOutput out, List<NodeHotThreads> nodes) throws IOException {
        out.writeStreamableList(nodes);
    }

}
