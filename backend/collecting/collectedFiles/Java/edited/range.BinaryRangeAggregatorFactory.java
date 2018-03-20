
package org.elasticsearch.search.aggregations.bucket.range;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories.Builder;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.aggregations.support.ValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class BinaryRangeAggregatorFactory
        extends ValuesSourceAggregatorFactory<ValuesSource.Bytes, BinaryRangeAggregatorFactory> {

    private final List<BinaryRangeAggregator.Range> ranges;
    private final boolean keyed;

    public BinaryRangeAggregatorFactory(String name,
            ValuesSourceConfig<ValuesSource.Bytes> config,
            List<BinaryRangeAggregator.Range> ranges, boolean keyed,
            SearchContext context,
            AggregatorFactory<?> parent, Builder subFactoriesBuilder,
            Map<String, Object> metaData) throws IOException {
        super(name, config, context, parent, subFactoriesBuilder, metaData);
        this.ranges = ranges;
        this.keyed = keyed;
    }

    @Override
    protected Aggregator createUnmapped(Aggregator parent,
            List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        return new BinaryRangeAggregator(name, factories, null, config.format(),
                ranges, keyed, context, parent, pipelineAggregators, metaData);
    }

    @Override
    protected Aggregator doCreateInternal(ValuesSource.Bytes valuesSource,
            Aggregator parent,
            boolean collectsFromSingleBucket,
            List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        return new BinaryRangeAggregator(name, factories, valuesSource, config.format(),
                ranges, keyed, context, parent, pipelineAggregators, metaData);
    }

}
