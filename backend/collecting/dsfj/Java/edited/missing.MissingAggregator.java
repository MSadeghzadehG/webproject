
package org.elasticsearch.search.aggregations.bucket.missing;

import org.apache.lucene.index.LeafReaderContext;
import org.elasticsearch.index.fielddata.DocValueBits;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.LeafBucketCollector;
import org.elasticsearch.search.aggregations.LeafBucketCollectorBase;
import org.elasticsearch.search.aggregations.bucket.BucketsAggregator;
import org.elasticsearch.search.aggregations.bucket.SingleBucketAggregator;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class MissingAggregator extends BucketsAggregator implements SingleBucketAggregator {

    private final ValuesSource valuesSource;

    public MissingAggregator(String name, AggregatorFactories factories, ValuesSource valuesSource,
            SearchContext aggregationContext, Aggregator parent, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) throws IOException {
        super(name, factories, aggregationContext, parent, pipelineAggregators, metaData);
        this.valuesSource = valuesSource;
    }

    @Override
    public LeafBucketCollector getLeafCollector(LeafReaderContext ctx,
            final LeafBucketCollector sub) throws IOException {
        final DocValueBits docsWithValue;
        if (valuesSource != null) {
            docsWithValue = valuesSource.docsWithValue(ctx);
        } else {
            docsWithValue = new DocValueBits() {
                @Override
                public boolean advanceExact(int doc) throws IOException {
                    return false;
                }
            };
        }
        return new LeafBucketCollectorBase(sub, docsWithValue) {
            @Override
            public void collect(int doc, long bucket) throws IOException {
                if (docsWithValue.advanceExact(doc) == false) {
                    collectBucket(sub, doc, bucket);
                }
            }
        };
    }

    @Override
    public InternalAggregation buildAggregation(long owningBucketOrdinal) throws IOException {
        return new InternalMissing(name, bucketDocCount(owningBucketOrdinal), bucketAggregations(owningBucketOrdinal), pipelineAggregators(),
                metaData());
    }

    @Override
    public InternalAggregation buildEmptyAggregation() {
        return new InternalMissing(name, 0, buildEmptySubAggregations(), pipelineAggregators(), metaData());
    }

}


