
package org.elasticsearch.search.aggregations.bucket.filter;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.search.Weight;
import org.apache.lucene.util.Bits;
import org.elasticsearch.common.lucene.Lucene;
import org.elasticsearch.search.aggregations.Aggregator;
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
import java.util.function.Supplier;


public class FilterAggregator extends BucketsAggregator implements SingleBucketAggregator {

    private final Supplier<Weight> filter;

    public FilterAggregator(String name,
                            Supplier<Weight> filter,
                            AggregatorFactories factories,
                            SearchContext context,
                            Aggregator parent, List<PipelineAggregator> pipelineAggregators,
                            Map<String, Object> metaData) throws IOException {
        super(name, factories, context, parent, pipelineAggregators, metaData);
        this.filter = filter;
    }

    @Override
    public LeafBucketCollector getLeafCollector(LeafReaderContext ctx,
            final LeafBucketCollector sub) throws IOException {
                final Bits bits = Lucene.asSequentialAccessBits(ctx.reader().maxDoc(), filter.get().scorerSupplier(ctx));
        return new LeafBucketCollectorBase(sub, null) {
            @Override
            public void collect(int doc, long bucket) throws IOException {
                if (bits.get(doc)) {
                    collectBucket(sub, doc, bucket);
                }
            }
        };
    }

    @Override
    public InternalAggregation buildAggregation(long owningBucketOrdinal) throws IOException {
        return new InternalFilter(name, bucketDocCount(owningBucketOrdinal), bucketAggregations(owningBucketOrdinal), pipelineAggregators(),
                metaData());
    }

    @Override
    public InternalAggregation buildEmptyAggregation() {
        return new InternalFilter(name, 0, buildEmptySubAggregations(), pipelineAggregators(), metaData());
    }
}


