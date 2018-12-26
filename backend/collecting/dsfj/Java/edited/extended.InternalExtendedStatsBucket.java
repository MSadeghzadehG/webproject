

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics.stats.extended;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.metrics.stats.extended.InternalExtendedStats;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class InternalExtendedStatsBucket extends InternalExtendedStats implements ExtendedStatsBucket {
    InternalExtendedStatsBucket(String name, long count, double sum, double min, double max, double sumOfSqrs, double sigma,
                                            DocValueFormat formatter, List<PipelineAggregator> pipelineAggregators,
                                            Map<String, Object> metaData) {
        super(name, count, sum, min, max, sumOfSqrs, sigma, formatter, pipelineAggregators, metaData);
    }

    
    public InternalExtendedStatsBucket(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return ExtendedStatsBucketPipelineAggregationBuilder.NAME;
    }

    @Override
    public org.elasticsearch.search.aggregations.metrics.stats.extended.InternalExtendedStats doReduce(
            List<InternalAggregation> aggregations, ReduceContext reduceContext) {
        throw new UnsupportedOperationException("Not supported");
    }
}
