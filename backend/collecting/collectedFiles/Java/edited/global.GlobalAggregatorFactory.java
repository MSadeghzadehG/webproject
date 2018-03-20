

package org.elasticsearch.search.aggregations.bucket.global;

import org.elasticsearch.search.aggregations.AggregationExecutionException;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class GlobalAggregatorFactory extends AggregatorFactory<GlobalAggregatorFactory> {

    public GlobalAggregatorFactory(String name, SearchContext context, AggregatorFactory<?> parent,
            AggregatorFactories.Builder subFactories, Map<String, Object> metaData) throws IOException {
        super(name, context, parent, subFactories, metaData);
    }

    @Override
    public Aggregator createInternal(Aggregator parent, boolean collectsFromSingleBucket, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        if (parent != null) {
            throw new AggregationExecutionException("Aggregation [" + parent.name() + "] cannot have a global " + "sub-aggregation [" + name
                    + "]. Global aggregations can only be defined as top level aggregations");
        }
        if (collectsFromSingleBucket == false) {
            throw new IllegalStateException();
        }
        return new GlobalAggregator(name, factories, context, pipelineAggregators, metaData);
    }
}
