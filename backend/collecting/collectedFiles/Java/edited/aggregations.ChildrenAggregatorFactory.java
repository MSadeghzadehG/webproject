

package org.elasticsearch.join.aggregations;

import org.apache.lucene.search.Query;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.NonCollectingAggregator;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource.Bytes.WithOrdinals;
import org.elasticsearch.search.aggregations.support.ValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class ChildrenAggregatorFactory
        extends ValuesSourceAggregatorFactory<WithOrdinals, ChildrenAggregatorFactory> {

    private final Query parentFilter;
    private final Query childFilter;

    public ChildrenAggregatorFactory(String name, ValuesSourceConfig<WithOrdinals> config,
                                     Query childFilter, Query parentFilter, SearchContext context, AggregatorFactory<?> parent,
                                     AggregatorFactories.Builder subFactoriesBuilder, Map<String, Object> metaData) throws IOException {
        super(name, config, context, parent, subFactoriesBuilder, metaData);
        this.childFilter = childFilter;
        this.parentFilter = parentFilter;
    }

    @Override
    protected Aggregator createUnmapped(Aggregator parent, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData)
            throws IOException {
        return new NonCollectingAggregator(name, context, parent, pipelineAggregators, metaData) {

            @Override
            public InternalAggregation buildEmptyAggregation() {
                return new InternalChildren(name, 0, buildEmptySubAggregations(), pipelineAggregators(), metaData());
            }

        };
    }

    @Override
    protected Aggregator doCreateInternal(WithOrdinals valuesSource, Aggregator parent,
            boolean collectsFromSingleBucket, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData)
                    throws IOException {
        long maxOrd = valuesSource.globalMaxOrd(context.searcher());
        return new ParentToChildrenAggregator(name, factories, context, parent, childFilter,
            parentFilter, valuesSource, maxOrd, pipelineAggregators, metaData);
    }
}
