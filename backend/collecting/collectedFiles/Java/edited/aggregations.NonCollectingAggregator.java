

package org.elasticsearch.search.aggregations;

import org.apache.lucene.index.LeafReaderContext;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public abstract class NonCollectingAggregator extends AggregatorBase {

    protected NonCollectingAggregator(String name, SearchContext context, Aggregator parent, AggregatorFactories subFactories,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        super(name, subFactories, context, parent, pipelineAggregators, metaData);
    }

    protected NonCollectingAggregator(String name, SearchContext context, Aggregator parent,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        this(name, context, parent, AggregatorFactories.EMPTY, pipelineAggregators, metaData);
    }

    @Override
    public final LeafBucketCollector getLeafCollector(LeafReaderContext reader, LeafBucketCollector sub) {
                return LeafBucketCollector.NO_OP_COLLECTOR;
    }

    @Override
    public final InternalAggregation buildAggregation(long owningBucketOrdinal) {
        return buildEmptyAggregation();
    }
}
