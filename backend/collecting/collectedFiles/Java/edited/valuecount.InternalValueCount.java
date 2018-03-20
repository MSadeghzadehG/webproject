
package org.elasticsearch.search.aggregations.metrics.valuecount;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.metrics.InternalNumericMetricsAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Objects;


public class InternalValueCount extends InternalNumericMetricsAggregation.SingleValue implements ValueCount {
    private final long value;

    public InternalValueCount(String name, long value, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, pipelineAggregators, metaData);
        this.value = value;
    }

    
    public InternalValueCount(StreamInput in) throws IOException {
        super(in);
        value = in.readVLong();
    }

    @Override
    protected void doWriteTo(StreamOutput out) throws IOException {
        out.writeVLong(value);
    }

    @Override
    public String getWriteableName() {
        return ValueCountAggregationBuilder.NAME;
    }

    @Override
    public long getValue() {
        return value;
    }

    @Override
    public double value() {
        return value;
    }

    @Override
    public InternalAggregation doReduce(List<InternalAggregation> aggregations, ReduceContext reduceContext) {
        long valueCount = 0;
        for (InternalAggregation aggregation : aggregations) {
            valueCount += ((InternalValueCount) aggregation).value;
        }
        return new InternalValueCount(name, valueCount, pipelineAggregators(), getMetaData());
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        builder.field(CommonFields.VALUE.getPreferredName(), value);
        return builder;
    }

    @Override
    public String toString() {
        return "count[" + value + "]";
    }

    @Override
    protected int doHashCode() {
        return Objects.hash(value);
    }

    @Override
    protected boolean doEquals(Object obj) {
        InternalValueCount that = (InternalValueCount) obj;
        return Objects.equals(this.value, that.value);
    }
}
