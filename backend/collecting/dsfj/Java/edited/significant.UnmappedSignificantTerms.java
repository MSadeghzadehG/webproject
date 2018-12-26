
package org.elasticsearch.search.aggregations.bucket.significant;

import org.apache.lucene.util.BytesRef;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.InternalAggregation;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.bucket.significant.heuristics.SignificanceHeuristic;
import org.elasticsearch.search.aggregations.bucket.terms.InternalTerms;
import org.elasticsearch.search.aggregations.bucket.terms.UnmappedTerms;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import static java.util.Collections.emptyIterator;
import static java.util.Collections.emptyList;


public class UnmappedSignificantTerms extends InternalSignificantTerms<UnmappedSignificantTerms, UnmappedSignificantTerms.Bucket> {

    public static final String NAME = "umsigterms";

    
    protected abstract static class Bucket extends InternalSignificantTerms.Bucket<Bucket> {
        private Bucket(BytesRef term, long subsetDf, long subsetSize, long supersetDf, long supersetSize, InternalAggregations aggregations,
                DocValueFormat format) {
            super(subsetDf, subsetSize, supersetDf, supersetSize, aggregations, format);
        }
    }

    public UnmappedSignificantTerms(String name, int requiredSize, long minDocCount, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        super(name, requiredSize, minDocCount, pipelineAggregators, metaData);
    }

    
    public UnmappedSignificantTerms(StreamInput in) throws IOException {
        super(in);
    }

    @Override
    protected void writeTermTypeInfoTo(StreamOutput out) throws IOException {
            }

    @Override
    public String getWriteableName() {
        return NAME;
    }

    @Override
    public String getType() {
        return SignificantStringTerms.NAME;
    }

    @Override
    public UnmappedSignificantTerms create(List<Bucket> buckets) {
        return new UnmappedSignificantTerms(name, requiredSize, minDocCount, pipelineAggregators(), metaData);
    }

    @Override
    public Bucket createBucket(InternalAggregations aggregations, Bucket prototype) {
        throw new UnsupportedOperationException("not supported for UnmappedSignificantTerms");
    }

    @Override
    protected UnmappedSignificantTerms create(long subsetSize, long supersetSize, List<Bucket> buckets) {
        throw new UnsupportedOperationException("not supported for UnmappedSignificantTerms");
    }

    @Override
    public InternalAggregation doReduce(List<InternalAggregation> aggregations, ReduceContext reduceContext) {
        for (InternalAggregation aggregation : aggregations) {
            if (!(aggregation instanceof UnmappedSignificantTerms)) {
                return aggregation.reduce(aggregations, reduceContext);
            }
        }
        return this;
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        builder.startArray(CommonFields.BUCKETS.getPreferredName()).endArray();
        return builder;
    }

    @Override
    protected Bucket[] createBucketsArray(int size) {
        return new Bucket[size];
    }

    @Override
    public Iterator<SignificantTerms.Bucket> iterator() {
        return emptyIterator();
    }

    @Override
    public List<Bucket> getBuckets() {
        return emptyList();
    }

    @Override
    public SignificantTerms.Bucket getBucketByKey(String term) {
        return null;
    }

    @Override
    protected SignificanceHeuristic getSignificanceHeuristic() {
        throw new UnsupportedOperationException();
    }

    @Override
    protected long getSubsetSize() {
        return 0;
    }

    @Override
    protected long getSupersetSize() {
        return 0;
    }
}
