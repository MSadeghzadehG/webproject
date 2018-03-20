

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.max;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.BucketHelpers.GapPolicy;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.BucketMetricsPipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.InternalBucketMetricValue;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public class MaxBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    private List<String> maxBucketKeys;
    private double maxValue;

    protected MaxBucketPipelineAggregator(String name, String[] bucketsPaths, GapPolicy gapPolicy, DocValueFormat formatter,
            Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
    }

    
    public MaxBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return MaxBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
        maxBucketKeys = new ArrayList<>();
        maxValue = Double.NEGATIVE_INFINITY;
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        if (bucketValue > maxValue) {
            maxBucketKeys.clear();
            maxBucketKeys.add(bucketKey);
            maxValue = bucketValue;
        } else if (bucketValue.equals(maxValue)) {
            maxBucketKeys.add(bucketKey);
        }
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {
        String[] keys = maxBucketKeys.toArray(new String[maxBucketKeys.size()]);
        return new InternalBucketMetricValue(name(), keys, maxValue, format, Collections.emptyList(), metaData());
    }

}
