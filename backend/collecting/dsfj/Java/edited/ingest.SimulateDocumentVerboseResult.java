
package org.elasticsearch.action.ingest;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public final class SimulateDocumentVerboseResult implements SimulateDocumentResult {
    private final List<SimulateProcessorResult> processorResults;

    public SimulateDocumentVerboseResult(List<SimulateProcessorResult> processorResults) {
        this.processorResults = processorResults;
    }

    
    public SimulateDocumentVerboseResult(StreamInput in) throws IOException {
        int size = in.readVInt();
        processorResults = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            processorResults.add(new SimulateProcessorResult(in));
        }
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeVInt(processorResults.size());
        for (SimulateProcessorResult result : processorResults) {
            result.writeTo(out);
        }
    }

    public List<SimulateProcessorResult> getProcessorResults() {
        return processorResults;
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        builder.startArray("processor_results");
        for (SimulateProcessorResult processorResult : processorResults) {
            processorResult.toXContent(builder, params);
        }
        builder.endArray();
        builder.endObject();
        return builder;
    }
}
