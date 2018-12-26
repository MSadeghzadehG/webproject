

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.stats;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.BucketHelpers.GapPolicy;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.BucketMetricsPipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class StatsBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    private double sum = 0;
    private long count = 0;
    private double min = Double.POSITIVE_INFINITY;
    private double max = Double.NEGATIVE_INFINITY;

    protected StatsBucketPipelineAggregator(String name, String[] bucketsPaths, GapPolicy gapPolicy, DocValueFormat formatter,
                                            Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
    }

    public StatsBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return StatsBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
        sum = 0;
        count = 0;
        min = Double.POSITIVE_INFINITY;
        max = Double.NEGATIVE_INFINITY;
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        sum += bucketValue;
        min = Math.min(min, bucketValue);
        max = Math.max(max, bucketValue);
        count += 1;
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {
        return new InternalStatsBucket(name(), count, sum, min, max, format, pipelineAggregators, metadata);
    }
}
