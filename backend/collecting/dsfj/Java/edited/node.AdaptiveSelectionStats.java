

package org.elasticsearch.node;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.common.util.set.Sets;
import org.elasticsearch.common.xcontent.ToXContent.Params;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;


public class AdaptiveSelectionStats implements Writeable, ToXContentFragment {

    private final Map<String, Long> clientOutgoingConnections;
    private final Map<String, ResponseCollectorService.ComputedNodeStats> nodeComputedStats;

    public AdaptiveSelectionStats(Map<String, Long> clientConnections,
                                  Map<String, ResponseCollectorService.ComputedNodeStats> nodeComputedStats) {
        this.clientOutgoingConnections = clientConnections;
        this.nodeComputedStats = nodeComputedStats;
    }

    public AdaptiveSelectionStats(StreamInput in) throws IOException {
        this.clientOutgoingConnections = in.readMap(StreamInput::readString, StreamInput::readLong);
        this.nodeComputedStats = in.readMap(StreamInput::readString, ResponseCollectorService.ComputedNodeStats::new);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeMap(this.clientOutgoingConnections, StreamOutput::writeString, StreamOutput::writeLong);
        out.writeMap(this.nodeComputedStats, StreamOutput::writeString, (stream, stats) -> stats.writeTo(stream));
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject("adaptive_selection");
        Set<String> allNodeIds = Sets.union(clientOutgoingConnections.keySet(), nodeComputedStats.keySet());
        for (String nodeId : allNodeIds) {
            builder.startObject(nodeId);
            ResponseCollectorService.ComputedNodeStats stats = nodeComputedStats.get(nodeId);
            if (stats != null) {
                long outgoingSearches = clientOutgoingConnections.getOrDefault(nodeId, 0L);
                builder.field("outgoing_searches", outgoingSearches);
                builder.field("avg_queue_size", stats.queueSize);
                builder.timeValueField("avg_service_time_ns", "avg_service_time", (long) stats.serviceTime, TimeUnit.NANOSECONDS);
                builder.timeValueField("avg_response_time_ns", "avg_response_time", (long) stats.responseTime, TimeUnit.NANOSECONDS);
                builder.field("rank", String.format(Locale.ROOT, "%.1f", stats.rank(outgoingSearches)));
            }
            builder.endObject();
        }
        builder.endObject();
        return builder;
    }

    
    public Map<String, Long> getOutgoingConnections() {
        return clientOutgoingConnections;
    }

    
    public Map<String, ResponseCollectorService.ComputedNodeStats> getComputedStats() {
        return nodeComputedStats;
    }

    
    public Map<String, Double> getRanks() {
        return nodeComputedStats.entrySet().stream()
                .collect(Collectors.toMap(Map.Entry::getKey,
                                e -> e.getValue().rank(clientOutgoingConnections.getOrDefault(e.getKey(), 0L))));
    }
}
