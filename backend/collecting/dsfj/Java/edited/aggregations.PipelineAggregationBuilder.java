
package org.elasticsearch.search.aggregations;

import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.stream.NamedWriteable;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.search.aggregations.AggregatorFactories.Builder;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.List;
import java.util.Map;


public abstract class PipelineAggregationBuilder implements NamedWriteable, BaseAggregationBuilder, ToXContentFragment {

    protected final String name;
    protected final String[] bucketsPaths;

    
    protected PipelineAggregationBuilder(String name, String[] bucketsPaths) {
        if (name == null) {
            throw new IllegalArgumentException("[name] must not be null: [" + name + "]");
        }
        if (bucketsPaths == null) {
            throw new IllegalArgumentException("[bucketsPaths] must not be null: [" + name + "]");
        }
        this.name = name;
        this.bucketsPaths = bucketsPaths;
    }

    
    public String getName() {
        return name;
    }

    
    public final String[] getBucketsPaths() {
        return bucketsPaths;
    }

    
    protected abstract void validate(AggregatorFactory<?> parent, List<AggregationBuilder> factories,
            List<PipelineAggregationBuilder> pipelineAggregatorFactories);

    
    protected abstract PipelineAggregator create() throws IOException;

    
    @Override
    public abstract PipelineAggregationBuilder setMetaData(Map<String, Object> metaData);

    @Override
    public PipelineAggregationBuilder subAggregations(Builder subFactories) {
        throw new IllegalArgumentException("Aggregation [" + name + "] cannot define sub-aggregations");
    }

    @Override
    public String toString() {
        return Strings.toString(this, true, true);
    }
}
