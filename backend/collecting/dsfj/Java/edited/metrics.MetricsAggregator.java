

package org.elasticsearch.search.aggregations.metrics;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorBase;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public abstract class MetricsAggregator extends AggregatorBase {
    
    protected MetricsAggregator(String name, SearchContext context, Aggregator parent, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        super(name, AggregatorFactories.EMPTY, context, parent, pipelineAggregators, metaData);
    }
}
