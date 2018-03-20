

package org.elasticsearch.search.aggregations.bucket.missing;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.aggregations.support.ValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class MissingAggregatorFactory extends ValuesSourceAggregatorFactory<ValuesSource, MissingAggregatorFactory> {

    public MissingAggregatorFactory(String name, ValuesSourceConfig<ValuesSource> config, SearchContext context,
            AggregatorFactory<?> parent, AggregatorFactories.Builder subFactoriesBuilder, Map<String, Object> metaData) throws IOException {
        super(name, config, context, parent, subFactoriesBuilder, metaData);
    }

    @Override
    protected MissingAggregator createUnmapped(Aggregator parent, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        return new MissingAggregator(name, factories, null, context, parent, pipelineAggregators, metaData);
    }

    @Override
    protected MissingAggregator doCreateInternal(ValuesSource valuesSource, Aggregator parent, boolean collectsFromSingleBucket,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        return new MissingAggregator(name, factories, valuesSource, context, parent, pipelineAggregators, metaData);
    }

}
