
package org.elasticsearch.search.aggregations.bucket.global;

import org.apache.lucene.index.LeafReaderContext;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.LeafBucketCollector;
import org.elasticsearch.search.aggregations.LeafBucketCollectorBase;
import org.elasticsearch.search.aggregations.bucket.BucketsAggregator;
import org.elasticsearch.search.aggregations.bucket.SingleBucketAggregator;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class GlobalAggregator extends BucketsAggregator implements SingleBucketAggregator {

    public GlobalAggregator(String name, AggregatorFactories subFactories, SearchContext aggregationContext,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        super(name, subFactories, aggregationContext, null, pipelineAggregators, metaData);
    }

    @Override
    public LeafBucketCollector getLeafCollector(LeafReaderContext ctx,
            final LeafBucketCollector sub) throws IOException {
        return new LeafBucketCollectorBase(sub, null) {
            @Override
            public void collect(int doc, long bucket) throws IOException {
                assert bucket == 0 : "global aggregator can only be a top level aggregator";
                collectBucket(sub, doc, bucket);
            }
        };
    }

    @Override
    public InternalAggregation buildAggregation(long owningBucketOrdinal) throws IOException {
        assert owningBucketOrdinal == 0 : "global aggregator can only be a top level aggregator";
        return new InternalGlobal(name, bucketDocCount(owningBucketOrdinal), bucketAggregations(owningBucketOrdinal), pipelineAggregators(),
                metaData());
    }

    @Override
    public InternalAggregation buildEmptyAggregation() {
        throw new UnsupportedOperationException(
                "global aggregations cannot serve as sub-aggregations, hence should never be called on #buildEmptyAggregations");
    }
}
