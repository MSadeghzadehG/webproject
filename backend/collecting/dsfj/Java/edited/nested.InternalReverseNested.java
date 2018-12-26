
package org.elasticsearch.search.aggregations.bucket.nested;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public class InternalReverseNested extends InternalSingleBucketAggregation implements ReverseNested {
    public InternalReverseNested(String name, long docCount, InternalAggregations aggregations,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) {
        super(name, docCount, aggregations, pipelineAggregators, metaData);
    }

    
    public InternalReverseNested(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return ReverseNestedAggregationBuilder.NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount, InternalAggregations subAggregations) {
        return new InternalReverseNested(name, docCount, subAggregations, pipelineAggregators(), getMetaData());
    }
}
