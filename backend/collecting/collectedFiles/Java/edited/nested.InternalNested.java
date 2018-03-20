
package org.elasticsearch.search.aggregations.bucket.nested;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public class InternalNested extends InternalSingleBucketAggregation implements Nested {
    InternalNested(String name, long docCount, InternalAggregations aggregations, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, docCount, aggregations, pipelineAggregators, metaData);
    }

    
    public InternalNested(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return NestedAggregationBuilder.NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount, InternalAggregations subAggregations) {
        return new InternalNested(name, docCount, subAggregations, pipelineAggregators(), getMetaData());
    }
}
