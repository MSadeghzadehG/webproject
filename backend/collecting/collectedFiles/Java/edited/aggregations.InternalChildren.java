

package org.elasticsearch.join.aggregations;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public class InternalChildren extends InternalSingleBucketAggregation implements Children {
    public InternalChildren(String name, long docCount, InternalAggregations aggregations, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, docCount, aggregations, pipelineAggregators, metaData);
    }

    
    public InternalChildren(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return ChildrenAggregationBuilder.NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount, InternalAggregations subAggregations) {
        return new InternalChildren(name, docCount, subAggregations, pipelineAggregators(), getMetaData());
    }
}
