

package org.elasticsearch.action.support.nodes;

import org.elasticsearch.action.ActionResponse;
import org.elasticsearch.action.FailedNodeException;
import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public abstract class BaseNodesResponse<TNodeResponse extends BaseNodeResponse> extends ActionResponse {

    private ClusterName clusterName;
    private List<FailedNodeException> failures;
    private List<TNodeResponse> nodes;
    private Map<String, TNodeResponse> nodesMap;

    protected BaseNodesResponse() {
    }

    protected BaseNodesResponse(ClusterName clusterName, List<TNodeResponse> nodes, List<FailedNodeException> failures) {
        this.clusterName = Objects.requireNonNull(clusterName);
        this.failures = Objects.requireNonNull(failures);
        this.nodes = Objects.requireNonNull(nodes);
    }

    
    public ClusterName getClusterName() {
        return clusterName;
    }

    
    public List<FailedNodeException> failures() {
        return failures;
    }

    
    public boolean hasFailures() {
        return failures.isEmpty() == false;
    }

    
    public List<TNodeResponse> getNodes() {
        return nodes;
    }

    
    public Map<String, TNodeResponse> getNodesMap() {
        if (nodesMap == null) {
            nodesMap = new HashMap<>();
            for (TNodeResponse nodeResponse : nodes) {
                nodesMap.put(nodeResponse.getNode().getId(), nodeResponse);
            }
        }
        return nodesMap;
    }

    @Override
    public void readFrom(StreamInput in) throws IOException {
        super.readFrom(in);
        clusterName = new ClusterName(in);
        nodes = readNodesFrom(in);
        failures = in.readList(FailedNodeException::new);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        clusterName.writeTo(out);
        writeNodesTo(out, nodes);
        out.writeList(failures);
    }

    
    protected abstract List<TNodeResponse> readNodesFrom(StreamInput in) throws IOException;

    
    protected abstract void writeNodesTo(StreamOutput out, List<TNodeResponse> nodes) throws IOException;

}
