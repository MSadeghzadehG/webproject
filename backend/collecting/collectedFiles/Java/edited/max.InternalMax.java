
package org.elasticsearch.search.aggregations.metrics.max;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.metrics.InternalNumericMetricsAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class InternalMax extends InternalNumericMetricsAggregation.SingleValue implements Max {
    private final double max;

    public InternalMax(String name, double max, DocValueFormat formatter, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, pipelineAggregators, metaData);
        this.format = formatter;
        this.max = max;
    }

    
    public InternalMax(StreamInput in) throws IOException {
        super(in);
        format = in.readNamedWriteable(DocValueFormat.class);
        max = in.readDouble();
    }

    @Override
    protected void doWriteTo(StreamOutput out) throws IOException {
        out.writeNamedWriteable(format);
        out.writeDouble(max);
    }

    @Override
    public String getWriteableName() {
        return MaxAggregationBuilder.NAME;
    }

    @Override
    public double value() {
        return max;
    }

    @Override
    public double getValue() {
        return max;
    }

    @Override
    public InternalMax doReduce(List<InternalAggregation> aggregations, ReduceContext reduceContext) {
        double max = Double.NEGATIVE_INFINITY;
        for (InternalAggregation aggregation : aggregations) {
            max = Math.max(max, ((InternalMax) aggregation).max);
        }
        return new InternalMax(name, max, format, pipelineAggregators(), getMetaData());
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        boolean hasValue = !Double.isInfinite(max);
        builder.field(CommonFields.VALUE.getPreferredName(), hasValue ? max : null);
        if (hasValue && format != DocValueFormat.RAW) {
            builder.field(CommonFields.VALUE_AS_STRING.getPreferredName(), format.format(max));
        }
        return builder;
    }

    @Override
    protected int doHashCode() {
        return Objects.hash(max);
    }

    @Override
    protected boolean doEquals(Object obj) {
        InternalMax other = (InternalMax) obj;
        return Objects.equals(max, other.max);
    }
}
