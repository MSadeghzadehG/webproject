

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.min;

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

public class MinBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    private List<String> minBucketKeys;
    private double minValue;

    protected MinBucketPipelineAggregator(String name, String[] bucketsPaths, GapPolicy gapPolicy, DocValueFormat formatter,
            Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
    }

    
    public MinBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return MinBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
        minBucketKeys = new ArrayList<>();
        minValue = Double.POSITIVE_INFINITY;
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        if (bucketValue < minValue) {
            minBucketKeys.clear();
            minBucketKeys.add(bucketKey);
            minValue = bucketValue;
        } else if (bucketValue.equals(minValue)) {
            minBucketKeys.add(bucketKey);
        }
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {
        String[] keys = minBucketKeys.toArray(new String[minBucketKeys.size()]);
        return new InternalBucketMetricValue(name(), keys, minValue, format, Collections.emptyList(), metaData());
    }

}
