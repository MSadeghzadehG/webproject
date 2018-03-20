
package org.elasticsearch.search.aggregations.bucket.global;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public class InternalGlobal extends InternalSingleBucketAggregation implements Global {
    InternalGlobal(String name, long docCount, InternalAggregations aggregations, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, docCount, aggregations, pipelineAggregators, metaData);
    }

    
    public InternalGlobal(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return GlobalAggregationBuilder.NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount, InternalAggregations subAggregations) {
        return new InternalGlobal(name, docCount, subAggregations, pipelineAggregators(), getMetaData());
    }
}
