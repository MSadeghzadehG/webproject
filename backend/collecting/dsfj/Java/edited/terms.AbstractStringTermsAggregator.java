

package org.elasticsearch.search.aggregations.bucket.terms;

import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.BucketOrder;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

import static java.util.Collections.emptyList;

abstract class AbstractStringTermsAggregator extends TermsAggregator {

    protected final boolean showTermDocCountError;

    AbstractStringTermsAggregator(String name, AggregatorFactories factories, SearchContext context, Aggregator parent,
            BucketOrder order, DocValueFormat format, BucketCountThresholds bucketCountThresholds, SubAggCollectionMode subAggCollectMode,
            boolean showTermDocCountError, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        super(name, factories, context, parent, bucketCountThresholds, order, format, subAggCollectMode, pipelineAggregators, metaData);
        this.showTermDocCountError = showTermDocCountError;
    }

    @Override
    public InternalAggregation buildEmptyAggregation() {
        return new StringTerms(name, order, bucketCountThresholds.getRequiredSize(), bucketCountThresholds.getMinDocCount(),
                pipelineAggregators(), metaData(), format, bucketCountThresholds.getShardSize(), showTermDocCountError, 0, emptyList(), 0);
    }

}
