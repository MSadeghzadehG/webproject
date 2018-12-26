

package org.elasticsearch.search.aggregations.bucket.filter;

import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Weight;
import org.elasticsearch.index.query.QueryBuilder;
import org.elasticsearch.search.aggregations.AggregationInitializationException;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class FilterAggregatorFactory extends AggregatorFactory<FilterAggregatorFactory> {

    private Weight weight;
    private Query filter;

    public FilterAggregatorFactory(String name, QueryBuilder filterBuilder, SearchContext context,
            AggregatorFactory<?> parent, AggregatorFactories.Builder subFactoriesBuilder, Map<String, Object> metaData) throws IOException {
        super(name, context, parent, subFactoriesBuilder, metaData);
        filter = filterBuilder.toFilter(context.getQueryShardContext());
    }

    
    public Weight getWeight() {
        if (weight == null) {
            IndexSearcher contextSearcher = context.searcher();
            try {
                weight = contextSearcher.createNormalizedWeight(filter, false);
            } catch (IOException e) {
                throw new AggregationInitializationException("Failed to initialse filter", e);
            }
        }
        return weight;
    }

    @Override
    public Aggregator createInternal(Aggregator parent, boolean collectsFromSingleBucket, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        return new FilterAggregator(name, () -> this.getWeight(), factories, context, parent, pipelineAggregators, metaData);
    }

}
