

package org.elasticsearch.search.aggregations.pipeline;


import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.io.stream.NamedWriteable;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.InternalAggregation.ReduceContext;
import org.elasticsearch.search.aggregations.PipelineAggregationBuilder;

import java.io.IOException;
import java.util.Map;

public abstract class PipelineAggregator implements NamedWriteable {
    
    @FunctionalInterface
    public interface Parser {
        ParseField BUCKETS_PATH = new ParseField("buckets_path");
        ParseField FORMAT = new ParseField("format");
        ParseField GAP_POLICY = new ParseField("gap_policy");

        
        PipelineAggregationBuilder parse(String pipelineAggregatorName, XContentParser parser)
                throws IOException;
    }

    private String name;
    private String[] bucketsPaths;
    private Map<String, Object> metaData;

    protected PipelineAggregator(String name, String[] bucketsPaths, Map<String, Object> metaData) {
        this.name = name;
        this.bucketsPaths = bucketsPaths;
        this.metaData = metaData;
    }

    
    protected PipelineAggregator(StreamInput in) throws IOException {
        name = in.readString();
        bucketsPaths = in.readStringArray();
        metaData = in.readMap();
    }

    @Override
    public final void writeTo(StreamOutput out) throws IOException {
        out.writeString(name);
        out.writeStringArray(bucketsPaths);
        out.writeMap(metaData);
        doWriteTo(out);
    }

    protected abstract void doWriteTo(StreamOutput out) throws IOException;

    public String name() {
        return name;
    }

    public String[] bucketsPaths() {
        return bucketsPaths;
    }

    public Map<String, Object> metaData() {
        return metaData;
    }

    public abstract InternalAggregation reduce(InternalAggregation aggregation, ReduceContext reduceContext);
}
