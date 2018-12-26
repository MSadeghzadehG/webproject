

package org.elasticsearch.search.aggregations.bucket.sampler;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class SamplerAggregatorFactory extends AggregatorFactory<SamplerAggregatorFactory> {

    private final int shardSize;

    SamplerAggregatorFactory(String name, int shardSize, SearchContext context, AggregatorFactory<?> parent,
            AggregatorFactories.Builder subFactories, Map<String, Object> metaData) throws IOException {
        super(name, context, parent, subFactories, metaData);
        this.shardSize = shardSize;
    }

    @Override
    public Aggregator createInternal(Aggregator parent, boolean collectsFromSingleBucket, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        return new SamplerAggregator(name, shardSize, factories, context, parent, pipelineAggregators, metaData);
    }

}
