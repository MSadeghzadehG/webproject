

package org.elasticsearch.index.rankeval;

import org.elasticsearch.common.io.stream.NamedWriteable;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.index.rankeval.RatedDocument.DocumentKey;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.SearchHits;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;


public interface EvaluationMetric extends ToXContentObject, NamedWriteable {

    
    EvalQueryQuality evaluate(String taskId, SearchHit[] hits, List<RatedDocument> ratedDocs);

    
    static List<RatedSearchHit> joinHitsWithRatings(SearchHit[] hits, List<RatedDocument> ratedDocs) {
        Map<DocumentKey, RatedDocument> ratedDocumentMap = ratedDocs.stream()
                .collect(Collectors.toMap(RatedDocument::getKey, item -> item));
        List<RatedSearchHit> ratedSearchHits = new ArrayList<>(hits.length);
        for (SearchHit hit : hits) {
            DocumentKey key = new DocumentKey(hit.getIndex(), hit.getId());
            RatedDocument ratedDoc = ratedDocumentMap.get(key);
            if (ratedDoc != null) {
                ratedSearchHits.add(new RatedSearchHit(hit, Optional.of(ratedDoc.getRating())));
            } else {
                ratedSearchHits.add(new RatedSearchHit(hit, Optional.empty()));
            }
        }
        return ratedSearchHits;
    }

    
    static List<DocumentKey> filterUnknownDocuments(List<RatedSearchHit> ratedHits) {
        List<DocumentKey> unknownDocs = ratedHits.stream().filter(hit -> hit.getRating().isPresent() == false)
                .map(hit -> new DocumentKey(hit.getSearchHit().getIndex(), hit.getSearchHit().getId())).collect(Collectors.toList());
        return unknownDocs;
    }

    
    default double combine(Collection<EvalQueryQuality> partialResults) {
        return partialResults.stream().mapToDouble(EvalQueryQuality::getQualityLevel).sum() / partialResults.size();
    }

    
    default Optional<Integer> forcedSearchSize() {
        return Optional.empty();
    }
}
