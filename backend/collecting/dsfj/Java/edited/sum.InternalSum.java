
package org.elasticsearch.search.aggregations.metrics.sum;

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

public class InternalSum extends InternalNumericMetricsAggregation.SingleValue implements Sum {
    private final double sum;

    public InternalSum(String name, double sum, DocValueFormat formatter, List<PipelineAggregator> pipelineAggregators,
                       Map<String, Object> metaData) {
        super(name, pipelineAggregators, metaData);
        this.sum = sum;
        this.format = formatter;
    }

    
    public InternalSum(StreamInput in) throws IOException {
        super(in);
        format = in.readNamedWriteable(DocValueFormat.class);
        sum = in.readDouble();
    }

    @Override
    protected void doWriteTo(StreamOutput out) throws IOException {
        out.writeNamedWriteable(format);
        out.writeDouble(sum);
    }

    @Override
    public String getWriteableName() {
        return SumAggregationBuilder.NAME;
    }

    @Override
    public double value() {
        return sum;
    }

    @Override
    public double getValue() {
        return sum;
    }

    @Override
    public InternalSum doReduce(List<InternalAggregation> aggregations, ReduceContext reduceContext) {
                        double sum = 0;
        double compensation = 0;
        for (InternalAggregation aggregation : aggregations) {
            double value = ((InternalSum) aggregation).sum;
            if (Double.isFinite(value) == false) {
                sum += value;
            } else if (Double.isFinite(sum)) {
                double corrected = value - compensation;
                double newSum = sum + corrected;
                compensation = (newSum - sum) - corrected;
                sum = newSum;
            }
        }
        return new InternalSum(name, sum, format, pipelineAggregators(), getMetaData());
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        builder.field(CommonFields.VALUE.getPreferredName(), sum);
        if (format != DocValueFormat.RAW) {
            builder.field(CommonFields.VALUE_AS_STRING.getPreferredName(), format.format(sum));
        }
        return builder;
    }

    @Override
    protected int doHashCode() {
        return Objects.hashCode(sum);
    }

    @Override
    protected boolean doEquals(Object obj) {
        InternalSum that = (InternalSum) obj;
        return Objects.equals(sum, that.sum);
    }
}
