

package org.elasticsearch.search.aggregations.bucket.filter;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class InternalFilter extends InternalSingleBucketAggregation implements Filter {
    InternalFilter(String name, long docCount, InternalAggregations subAggregations, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, docCount, subAggregations, pipelineAggregators, metaData);
    }

    
    public InternalFilter(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return FilterAggregationBuilder.NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount, InternalAggregations subAggregations) {
        return new InternalFilter(name, docCount, subAggregations, pipelineAggregators(), getMetaData());
    }
}