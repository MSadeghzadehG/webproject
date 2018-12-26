

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.percentile;

import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.BucketHelpers.GapPolicy;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.BucketMetricsPipelineAggregator;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public class PercentilesBucketPipelineAggregator extends BucketMetricsPipelineAggregator {
    public final ParseField PERCENTS_FIELD = new ParseField("percents");

    private final double[] percents;
    private List<Double> data;

    protected PercentilesBucketPipelineAggregator(String name, double[] percents, String[] bucketsPaths, GapPolicy gapPolicy,
                                                  DocValueFormat formatter, Map<String, Object> metaData) {
        super(name, bucketsPaths, gapPolicy, formatter, metaData);
        this.percents = percents;
    }

    
    public PercentilesBucketPipelineAggregator(StreamInput in) throws IOException {
        super(in);
        percents = in.readDoubleArray();
    }

    @Override
    public void innerWriteTo(StreamOutput out) throws IOException {
        out.writeDoubleArray(percents);
    }

    @Override
    public String getWriteableName() {
        return PercentilesBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    protected void preCollection() {
       data = new ArrayList<>(1024);
    }

    @Override
    protected void collectBucketValue(String bucketKey, Double bucketValue) {
        data.add(bucketValue);
    }

    @Override
    protected InternalAggregation buildAggregation(List<PipelineAggregator> pipelineAggregators, Map<String, Object> metadata) {

                        Collections.sort(data);

        double[] percentiles = new double[percents.length];
        if (data.size() == 0) {
            for (int i = 0; i < percents.length; i++) {
                percentiles[i] = Double.NaN;
            }
        } else {
            for (int i = 0; i < percents.length; i++) {
                int index = (int) Math.round((percents[i] / 100.0) * (data.size() - 1));
                percentiles[i] = data.get(index);
            }
        }

        
        return new InternalPercentilesBucket(name(), percents, percentiles, format, pipelineAggregators, metadata);
    }
}
