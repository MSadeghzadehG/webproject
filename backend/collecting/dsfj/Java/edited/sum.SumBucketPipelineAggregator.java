

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.sum;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.BucketHelpers.GapPolicy;
import org.elasticsearch.search.aggregations.pipeline.InternalSimpleValue;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.BucketMetricsPipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class SumBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    private double sum = 0;

    protected SumBucketPipelineAggregator(String name, String[] bucketsPaths, GapPolicy gapPolicy, DocValueFormat formatter,
            Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
    }

    
    public SumBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return SumBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
        sum = 0;
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        sum += bucketValue;
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {
        return new InternalSimpleValue(name(), sum, format, pipelineAggregators, metadata);
    }

}
