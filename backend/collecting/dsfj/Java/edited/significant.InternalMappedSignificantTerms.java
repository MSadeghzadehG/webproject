

package org.elasticsearch.search.aggregations.bucket.significant;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.search.DocValueFormat;
import org.elasticsearch.search.aggregations.bucket.significant.heuristics.SignificanceHeuristic;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.function.Function;
import java.util.stream.Collectors;

public abstract class InternalMappedSignificantTerms<
            A extends InternalMappedSignificantTerms<A, B>, 
            B extends InternalSignificantTerms.Bucket<B>>
        extends InternalSignificantTerms<A, B> {

    protected final DocValueFormat format;
    protected final long subsetSize;
    protected final long supersetSize;
    protected final SignificanceHeuristic significanceHeuristic;
    protected final List<B> buckets;
    protected Map<String, B> bucketMap;

    protected InternalMappedSignificantTerms(String name, int requiredSize, long minDocCount, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData, DocValueFormat format, long subsetSize, long supersetSize,
            SignificanceHeuristic significanceHeuristic, List<B> buckets) {
        super(name, requiredSize, minDocCount, pipelineAggregators, metaData);
        this.format = format;
        this.buckets = buckets;
        this.subsetSize = subsetSize;
        this.supersetSize = supersetSize;
        this.significanceHeuristic = significanceHeuristic;
    }

    protected InternalMappedSignificantTerms(StreamInput in, Bucket.Reader<B> bucketReader) throws IOException {
        super(in);
        format = in.readNamedWriteable(DocValueFormat.class);
        subsetSize = in.readVLong();
        supersetSize = in.readVLong();
        significanceHeuristic = in.readNamedWriteable(SignificanceHeuristic.class);
        buckets = in.readList(stream -> bucketReader.read(stream, subsetSize, supersetSize, format));
    }

    @Override
    protected final void writeTermTypeInfoTo(StreamOutput out) throws IOException {
        out.writeNamedWriteable(format);
        out.writeVLong(subsetSize);
        out.writeVLong(supersetSize);
        out.writeNamedWriteable(significanceHeuristic);
        out.writeList(buckets);
    }

    @Override
    public Iterator<SignificantTerms.Bucket> iterator() {
        return buckets.stream().map(bucket -> (SignificantTerms.Bucket) bucket).collect(Collectors.toList()).iterator();
    }

    @Override
    public List<B> getBuckets() {
        return buckets;
    }

    @Override
    public B getBucketByKey(String term) {
        if (bucketMap == null) {
            bucketMap = buckets.stream().collect(Collectors.toMap(InternalSignificantTerms.Bucket::getKeyAsString, Function.identity()));
        }
        return bucketMap.get(term);
    }

    @Override
    protected long getSubsetSize() {
        return subsetSize;
    }

    @Override
    protected long getSupersetSize() {
        return supersetSize;
    }

    @Override
    protected SignificanceHeuristic getSignificanceHeuristic() {
        return significanceHeuristic;
    }

    @Override
    protected boolean doEquals(Object obj) {
        InternalMappedSignificantTerms<?, ?> that = (InternalMappedSignificantTerms<?, ?>) obj;
        return super.doEquals(obj)
                && Objects.equals(format, that.format)
                && subsetSize == that.subsetSize
                && supersetSize == that.supersetSize
                && Objects.equals(significanceHeuristic, that.significanceHeuristic)
                && Objects.equals(buckets, that.buckets)
                && Objects.equals(bucketMap, that.bucketMap);
    }

    @Override
    protected int doHashCode() {
        return Objects.hash(super.doHashCode(), format, subsetSize, supersetSize, significanceHeuristic, buckets, bucketMap);
    }

    @Override
    public XContentBuilder doXContentBody(XContentBuilder builder, Params params) throws IOException {
        builder.field(CommonFields.DOC_COUNT.getPreferredName(), subsetSize);
        builder.field(BG_COUNT, supersetSize);
        builder.startArray(CommonFields.BUCKETS.getPreferredName());
        for (Bucket bucket : buckets) {
                                    if (bucket.subsetDf >= minDocCount) {
                bucket.toXContent(builder, params);
            }
        }
        builder.endArray();
        return builder;
    }
}
