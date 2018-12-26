
package org.elasticsearch.search.aggregations.metrics.percentiles.tdigest;

import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource.Numeric;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class TDigestPercentilesAggregator extends AbstractTDigestPercentilesAggregator {

    public TDigestPercentilesAggregator(String name, Numeric valuesSource, SearchContext context,
            Aggregator parent, double[] percents,
            double compression, boolean keyed, DocValueFormat formatter, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        super(name, valuesSource, context, parent, percents, compression, keyed, formatter, pipelineAggregators, metaData);
    }

    @Override
    public InternalAggregation buildAggregation(long owningBucketOrdinal) {
        TDigestState state = getState(owningBucketOrdinal);
        if (state == null) {
            return buildEmptyAggregation();
        } else {
            return new InternalTDigestPercentiles(name, keys, state, keyed, formatter, pipelineAggregators(), metaData());
        }
    }

    @Override
    public double metric(String name, long bucketOrd) {
        TDigestState state = getState(bucketOrd);
        if (state == null) {
            return Double.NaN;
        } else {
            return state.quantile(Double.parseDouble(name) / 100);
        }
    }

    @Override
    public InternalAggregation buildEmptyAggregation() {
        return new InternalTDigestPercentiles(name, keys, new TDigestState(compression), keyed, formatter, pipelineAggregators(), metaData());
    }
}
