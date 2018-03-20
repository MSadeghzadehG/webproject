

package org.elasticsearch.cluster.routing.allocation;

import org.elasticsearch.cluster.routing.allocation.decider.Decision;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContent.Params;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;


public class RoutingExplanations implements ToXContentFragment {
    private final List<RerouteExplanation> explanations;

    public RoutingExplanations() {
        this.explanations = new ArrayList<>();
    }

    public RoutingExplanations add(RerouteExplanation explanation) {
        this.explanations.add(explanation);
        return this;
    }

    public List<RerouteExplanation> explanations() {
        return this.explanations;
    }

    
    public List<String> getYesDecisionMessages() {
        return explanations().stream()
            .filter(explanation -> explanation.decisions().type().equals(Decision.Type.YES))
            .map(explanation -> explanation.command().getMessage())
            .filter(Optional::isPresent)
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    
    public static RoutingExplanations readFrom(StreamInput in) throws IOException {
        int exCount = in.readVInt();
        RoutingExplanations exp = new RoutingExplanations();
        for (int i = 0; i < exCount; i++) {
            RerouteExplanation explanation = RerouteExplanation.readFrom(in);
            exp.add(explanation);
        }
        return exp;
    }

    
    public static void writeTo(RoutingExplanations explanations, StreamOutput out) throws IOException {
        out.writeVInt(explanations.explanations.size());
        for (RerouteExplanation explanation : explanations.explanations) {
            RerouteExplanation.writeTo(explanation, out);
        }
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startArray("explanations");
        for (RerouteExplanation explanation : explanations) {
            explanation.toXContent(builder, params);
        }
        builder.endArray();
        return builder;
    }
}
