

package org.elasticsearch.search.aggregations.metrics;

import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.ParsedAggregation;
import org.elasticsearch.search.aggregations.metrics.stats.InternalStats;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.stats.InternalStatsBucket;
import org.elasticsearch.search.aggregations.pipeline.bucketmetrics.stats.ParsedStatsBucket;

import java.util.Collections;
import java.util.List;
import java.util.Map;

public class InternalStatsBucketTests extends InternalStatsTests {

    @Override
    protected InternalStatsBucket createInstance(String name, long count, double sum, double min, double max,
            DocValueFormat formatter, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) {
        return new InternalStatsBucket(name, count, sum, min, max, formatter, pipelineAggregators, metaData);
    }

    @Override
    public void testReduceRandom() {
        expectThrows(UnsupportedOperationException.class,
                () -> createTestInstance("name", Collections.emptyList(), null).reduce(null, null));
    }

    @Override
    protected void assertReduced(InternalStats reduced, List<InternalStats> inputs) {
            }

    @Override
    protected Writeable.Reader<InternalStats> instanceReader() {
        return InternalStatsBucket::new;
    }

    @Override
    protected void assertFromXContent(InternalStats aggregation, ParsedAggregation parsedAggregation) {
        super.assertFromXContent(aggregation, parsedAggregation);
        assertTrue(parsedAggregation instanceof ParsedStatsBucket);
    }
}
