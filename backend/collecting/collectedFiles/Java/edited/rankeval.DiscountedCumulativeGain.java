

package org.elasticsearch.index.rankeval;

import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ConstructingObjectParser;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.search.SearchHit;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

import static org.elasticsearch.common.xcontent.ConstructingObjectParser.optionalConstructorArg;
import static org.elasticsearch.index.rankeval.EvaluationMetric.joinHitsWithRatings;


public class DiscountedCumulativeGain implements EvaluationMetric {

    
    private final boolean normalize;

    
    private static final int DEFAULT_K = 10;

    
    private final int k;

    
    private final Integer unknownDocRating;

    public static final String NAME = "dcg";
    private static final double LOG2 = Math.log(2.0);

    public DiscountedCumulativeGain() {
        this(false, null, DEFAULT_K);
    }

    
    public DiscountedCumulativeGain(boolean normalize, Integer unknownDocRating, int k) {
        this.normalize = normalize;
        this.unknownDocRating = unknownDocRating;
        this.k = k;
    }

    DiscountedCumulativeGain(StreamInput in) throws IOException {
        normalize = in.readBoolean();
        unknownDocRating = in.readOptionalVInt();
        k = in.readVInt();
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeBoolean(normalize);
        out.writeOptionalVInt(unknownDocRating);
        out.writeVInt(k);
    }

    @Override
    public String getWriteableName() {
        return NAME;
    }

    boolean getNormalize() {
        return this.normalize;
    }

    int getK() {
        return this.k;
    }

    
    public Integer getUnknownDocRating() {
        return this.unknownDocRating;
    }


    @Override
    public Optional<Integer> forcedSearchSize() {
        return Optional.of(k);
    }

    @Override
    public EvalQueryQuality evaluate(String taskId, SearchHit[] hits,
            List<RatedDocument> ratedDocs) {
        List<Integer> allRatings = ratedDocs.stream().mapToInt(RatedDocument::getRating).boxed()
                .collect(Collectors.toList());
        List<RatedSearchHit> ratedHits = joinHitsWithRatings(hits, ratedDocs);
        List<Integer> ratingsInSearchHits = new ArrayList<>(ratedHits.size());
        for (RatedSearchHit hit : ratedHits) {
                                                            ratingsInSearchHits.add(hit.getRating().orElse(unknownDocRating));
        }
        double dcg = computeDCG(ratingsInSearchHits);

        if (normalize) {
            Collections.sort(allRatings, Comparator.nullsLast(Collections.reverseOrder()));
            double idcg = computeDCG(
                    allRatings.subList(0, Math.min(ratingsInSearchHits.size(), allRatings.size())));
            dcg = dcg / idcg;
        }
        EvalQueryQuality evalQueryQuality = new EvalQueryQuality(taskId, dcg);
        evalQueryQuality.addHitsAndRatings(ratedHits);
        return evalQueryQuality;
    }

    private static double computeDCG(List<Integer> ratings) {
        int rank = 1;
        double dcg = 0;
        for (Integer rating : ratings) {
            if (rating != null) {
                dcg += (Math.pow(2, rating) - 1) / ((Math.log(rank + 1) / LOG2));
            }
            rank++;
        }
        return dcg;
    }

    private static final ParseField K_FIELD = new ParseField("k");
    private static final ParseField NORMALIZE_FIELD = new ParseField("normalize");
    private static final ParseField UNKNOWN_DOC_RATING_FIELD = new ParseField("unknown_doc_rating");
    private static final ConstructingObjectParser<DiscountedCumulativeGain, Void> PARSER = new ConstructingObjectParser<>("dcg_at", false,
            args -> {
                Boolean normalized = (Boolean) args[0];
                Integer optK = (Integer) args[2];
                return new DiscountedCumulativeGain(normalized == null ? false : normalized, (Integer) args[1],
                        optK == null ? DEFAULT_K : optK);
            });

    static {
        PARSER.declareBoolean(optionalConstructorArg(), NORMALIZE_FIELD);
        PARSER.declareInt(optionalConstructorArg(), UNKNOWN_DOC_RATING_FIELD);
        PARSER.declareInt(optionalConstructorArg(), K_FIELD);
    }

    public static DiscountedCumulativeGain fromXContent(XContentParser parser) {
        return PARSER.apply(parser, null);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        builder.startObject(NAME);
        builder.field(NORMALIZE_FIELD.getPreferredName(), this.normalize);
        if (unknownDocRating != null) {
            builder.field(UNKNOWN_DOC_RATING_FIELD.getPreferredName(), this.unknownDocRating);
        }
        builder.field(K_FIELD.getPreferredName(), this.k);
        builder.endObject();
        builder.endObject();
        return builder;
    }

    @Override
    public final boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        DiscountedCumulativeGain other = (DiscountedCumulativeGain) obj;
        return Objects.equals(normalize, other.normalize)
                && Objects.equals(unknownDocRating, other.unknownDocRating)
                && Objects.equals(k, other.k);
    }

    @Override
    public final int hashCode() {
        return Objects.hash(normalize, unknownDocRating, k);
    }
}
