

package org.elasticsearch.search.aggregations.pipeline.bucketmetrics;

import org.elasticsearch.search.aggregations.AggregationBuilder;
import org.elasticsearch.search.aggregations.bucket.global.GlobalAggregationBuilder;
import org.elasticsearch.search.aggregations.bucket.terms.TermsAggregationBuilder;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.avg.AvgBucketPipelineAggregationBuilder;
import org.elasticsearch.search.aggregations.support.ValueType;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class AvgBucketTests extends AbstractBucketMetricsTestCase<AvgBucketPipelineAggregationBuilder> {

    @Override
    protected AvgBucketPipelineAggregationBuilder doCreateTestAggregatorFactory(String name, String bucketsPath) {
        return new AvgBucketPipelineAggregationBuilder(name, bucketsPath);
    }

    public void testValidate() {
        AggregationBuilder singleBucketAgg = new GlobalAggregationBuilder("global");
        AggregationBuilder multiBucketAgg = new TermsAggregationBuilder("terms", ValueType.STRING);
        final List<AggregationBuilder> aggBuilders = new ArrayList<>();
        aggBuilders.add(singleBucketAgg);
        aggBuilders.add(multiBucketAgg);

                final AvgBucketPipelineAggregationBuilder builder = new AvgBucketPipelineAggregationBuilder("name", "invalid_agg>metric");
        IllegalArgumentException ex = expectThrows(IllegalArgumentException.class,
                () -> builder.validate(null, aggBuilders, Collections.emptyList()));
        assertEquals(PipelineAggregator.Parser.BUCKETS_PATH.getPreferredName()
                + " aggregation does not exist for aggregation [name]: invalid_agg>metric", ex.getMessage());
        
                AvgBucketPipelineAggregationBuilder builder2 = new AvgBucketPipelineAggregationBuilder("name", "global>metric");
        ex = expectThrows(IllegalArgumentException.class, () -> builder2.validate(null, aggBuilders, Collections.emptyList()));
        assertEquals("The first aggregation in " + PipelineAggregator.Parser.BUCKETS_PATH.getPreferredName()
                + " must be a multi-bucket aggregation for aggregation [name] found :" + GlobalAggregationBuilder.class.getName()
                + " for buckets path: global>metric", ex.getMessage());
        
                AvgBucketPipelineAggregationBuilder builder3 = new AvgBucketPipelineAggregationBuilder("name", "terms>metric");
        builder3.validate(null, aggBuilders, Collections.emptyList());
        
    }

}
