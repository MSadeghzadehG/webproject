
package org.elasticsearch.search.aggregations.matrix.stats;

import org.elasticsearch.search.MultiValueMode;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.MultiValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

final class MatrixStatsAggregatorFactory
    extends MultiValuesSourceAggregatorFactory<ValuesSource.Numeric, MatrixStatsAggregatorFactory> {

    private final MultiValueMode multiValueMode;

    MatrixStatsAggregatorFactory(String name,
            Map<String, ValuesSourceConfig<ValuesSource.Numeric>> configs, MultiValueMode multiValueMode,
            SearchContext context, AggregatorFactory<?> parent, AggregatorFactories.Builder subFactoriesBuilder,
            Map<String, Object> metaData) throws IOException {
        super(name, configs, context, parent, subFactoriesBuilder, metaData);
        this.multiValueMode = multiValueMode;
    }

    @Override
    protected Aggregator createUnmapped(Aggregator parent, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData)
        throws IOException {
        return new MatrixStatsAggregator(name, null, context, parent, multiValueMode, pipelineAggregators, metaData);
    }

    @Override
    protected Aggregator doCreateInternal(Map<String, ValuesSource.Numeric> valuesSources, Aggregator parent,
                                          boolean collectsFromSingleBucket, List<PipelineAggregator> pipelineAggregators,
                                          Map<String, Object> metaData) throws IOException {
        return new MatrixStatsAggregator(name, valuesSources, context, parent, multiValueMode, pipelineAggregators, metaData);
    }
}
