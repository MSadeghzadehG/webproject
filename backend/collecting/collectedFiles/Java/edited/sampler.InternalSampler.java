
package org.elasticsearch.search.aggregations.bucket.sampler;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.InternalSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class InternalSampler extends InternalSingleBucketAggregation implements Sampler {
    public static final String NAME = "mapped_sampler";
        public static final String PARSER_NAME = "sampler";

    InternalSampler(String name, long docCount, InternalAggregations subAggregations, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, docCount, subAggregations, pipelineAggregators, metaData);
    }

    
    public InternalSampler(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    public String getWriteableName() {
        return NAME;
    }

    @Override
    public String getType() {
        return PARSER_NAME;
    }

    @Override
    protected InternalSingleBucketAggregation newAggregation(String name, long docCount,
            InternalAggregations subAggregations) {
        return new InternalSampler(name, docCount, subAggregations, pipelineAggregators(), metaData);
    }
}