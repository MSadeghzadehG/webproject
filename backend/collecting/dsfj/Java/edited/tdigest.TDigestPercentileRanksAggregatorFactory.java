

package org.elasticsearch.search.aggregations.metrics.percentiles.tdigest;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.aggregations.support.ValuesSource.Numeric;
import org.elasticsearch.search.aggregations.support.ValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class TDigestPercentileRanksAggregatorFactory
        extends ValuesSourceAggregatorFactory<ValuesSource.Numeric, TDigestPercentileRanksAggregatorFactory> {

    private final double[] percents;
    private final double compression;
    private final boolean keyed;

    public TDigestPercentileRanksAggregatorFactory(String name, ValuesSourceConfig<Numeric> config, double[] percents,
            double compression, boolean keyed, SearchContext context, AggregatorFactory<?> parent,
            AggregatorFactories.Builder subFactoriesBuilder, Map<String, Object> metaData) throws IOException {
        super(name, config, context, parent, subFactoriesBuilder, metaData);
        this.percents = percents;
        this.compression = compression;
        this.keyed = keyed;
    }

    @Override
    protected Aggregator createUnmapped(Aggregator parent, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData)
            throws IOException {
        return new TDigestPercentileRanksAggregator(name, null, context, parent, percents, compression, keyed, config.format(),
                pipelineAggregators, metaData);
    }

    @Override
    protected Aggregator doCreateInternal(Numeric valuesSource, Aggregator parent, boolean collectsFromSingleBucket,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        return new TDigestPercentileRanksAggregator(name, valuesSource, context, parent, percents, compression, keyed, config.format(),
                pipelineAggregators, metaData);
    }

}
