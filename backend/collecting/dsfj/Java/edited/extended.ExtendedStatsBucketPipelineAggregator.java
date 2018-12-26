

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.stats.extended;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.BucketHelpers.GapPolicy;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.BucketMetricsPipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class ExtendedStatsBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    private final double sigma;
    private double sum = 0;
    private long count = 0;
    private double min = Double.POSITIVE_INFINITY;
    private double max = Double.NEGATIVE_INFINITY;
    private double sumOfSqrs = 1;

    protected ExtendedStatsBucketPipelineAggregator(String name, String[] bucketsPaths, double sigma, GapPolicy gapPolicy,
                                                    DocValueFormat formatter, Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
        this.sigma = sigma;
    }

    
    public ExtendedStatsBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
        sigma = in.readDouble();
    }

    @Override
    protected void innerWriteTo(StreamOutput out) throws IOException {
        out.writeDouble(sigma);
    }

    @Override
    public String getWriteableName() {
        return ExtendedStatsBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
        sum = 0;
        count = 0;
        min = Double.POSITIVE_INFINITY;
        max = Double.NEGATIVE_INFINITY;
        sumOfSqrs = 0;
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        sum += bucketValue;
        min = Math.min(min, bucketValue);
        max = Math.max(max, bucketValue);
        count += 1;
        sumOfSqrs += bucketValue * bucketValue;
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {
        return new InternalExtendedStatsBucket(name(), count, sum, min, max, sumOfSqrs, sigma, format, pipelineAggregators, metadata);
    }
}
