

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics;

import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.metrics.InternalNumericMetricsAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class InternalBucketMetricValue extends InternalNumericMetricsAggregation.SingleValue implements BucketMetricValue {
    public static final String NAME = "bucket_metric_value";
    static final ParseField KEYS_FIELD = new ParseField("keys");

    private double value;
    private String[] keys;

    public InternalBucketMetricValue(String name, String[] keys, double value, DocValueFormat formatter,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) {
        super(name, pipelineAggregators, metaData);
        this.keys = keys;
        this.value = value;
        this.format = formatter;
    }

    
    public InternalBucketMetricValue(StreamInput in) throws IOException {
        super(in);
        format = in.readNamedWriteable(DocValueFormat.class);
        value = in.readDouble();
        keys = in.readStringArray();
    }

    @Override
    protected void doWriteTo(StreamOutput out) throws IOException {
        out.writeNamedWriteable(format);
        out.writeDouble(value);
        out.writeStringArray(keys);
    }

    @Override
    public String getWriteableName() {
        return NAME;
    }

    @Override
    public double value() {
        return value;
    }

    @Override
    public String[] keys() {
        return keys;
    }

    DocValueFormat formatter() {
        return format;
    }

    @Override
    public InternalAggregation doReduce(List<InternalAggregation> aggregations, ReduceContext reduceContext) {
        throw new UnsupportedOperationException("Not supported");
    }

    @Override
    public Object getProperty(List<String> path) {
        if (path.isEmpty()) {
            return this;
        } else if (path.size() == 1 && "value".equals(path.get(0))) {
            return value();
        } else if (path.size() == 1 && KEYS_FIELD.getPreferredName().equals(path.get(0))) {
            return keys();
        } else {
            throw new IllegalArgumentException("path not supported for [" + getName() + "]: " + path);
        }
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        boolean hasValue = !Double.isInfinite(value);
        builder.field(CommonFields.VALUE.getPreferredName(), hasValue ? value : null);
        if (hasValue && format != DocValueFormat.RAW) {
            builder.field(CommonFields.VALUE_AS_STRING.getPreferredName(), format.format(value));
        }
        builder.startArray(KEYS_FIELD.getPreferredName());
        for (String key : keys) {
            builder.value(key);
        }
        builder.endArray();
        return builder;
    }

    @Override
    protected int doHashCode() {
        return Objects.hash(value, Arrays.hashCode(keys));
    }

    @Override
    protected boolean doEquals(Object obj) {
        InternalBucketMetricValue other = (InternalBucketMetricValue) obj;
        return Objects.equals(value, other.value)
                && Arrays.equals(keys, other.keys);
    }
}
