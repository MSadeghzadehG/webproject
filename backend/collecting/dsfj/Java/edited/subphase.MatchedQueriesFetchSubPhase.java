
package org.elasticsearch.search.fetch.subphase;

import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.index.ReaderUtil;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.ScorerSupplier;
import org.apache.lucene.search.Weight;
import org.apache.lucene.util.Bits;
import org.elasticsearch.ExceptionsHelper;
import org.elasticsearch.common.lucene.Lucene;
import org.elasticsearch.search.fetch.FetchSubPhase;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.internal.SearchContext;
import org.elasticsearch.search.internal.SearchContext.Lifetime;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class MatchedQueriesFetchSubPhase implements FetchSubPhase {

    @Override
    public void hitsExecute(SearchContext context, SearchHit[] hits) {
        if (hits.length == 0 ||
                        context.parsedQuery() == null) {
            return;
        }
        hits = hits.clone();         Arrays.sort(hits, (a, b) -> Integer.compare(a.docId(), b.docId()));
        @SuppressWarnings("unchecked")
        List<String>[] matchedQueries = new List[hits.length];
        for (int i = 0; i < matchedQueries.length; ++i) {
            matchedQueries[i] = new ArrayList<>();
        }

        Map<String, Query> namedQueries = new HashMap<>(context.parsedQuery().namedFilters());
        if (context.parsedPostFilter() != null) {
            namedQueries.putAll(context.parsedPostFilter().namedFilters());
        }

        try {
            for (Map.Entry<String, Query> entry : namedQueries.entrySet()) {
                String name = entry.getKey();
                Query query = entry.getValue();
                int readerIndex = -1;
                int docBase = -1;
                Weight weight = context.searcher().createNormalizedWeight(query, false);
                Bits matchingDocs = null;
                final IndexReader indexReader = context.searcher().getIndexReader();
                for (int i = 0; i < hits.length; ++i) {
                    SearchHit hit = hits[i];
                    int hitReaderIndex = ReaderUtil.subIndex(hit.docId(), indexReader.leaves());
                    if (readerIndex != hitReaderIndex) {
                        readerIndex = hitReaderIndex;
                        LeafReaderContext ctx = indexReader.leaves().get(readerIndex);
                        docBase = ctx.docBase;
                                                ScorerSupplier scorerSupplier = weight.scorerSupplier(ctx);
                        matchingDocs = Lucene.asSequentialAccessBits(ctx.reader().maxDoc(), scorerSupplier);
                    }
                    if (matchingDocs.get(hit.docId() - docBase)) {
                        matchedQueries[i].add(name);
                    }
                }
            }
            for (int i = 0; i < hits.length; ++i) {
                hits[i].matchedQueries(matchedQueries[i].toArray(new String[matchedQueries[i].size()]));
            }
        } catch (IOException e) {
            throw ExceptionsHelper.convertToElastic(e);
        } finally {
            context.clearReleasables(Lifetime.COLLECTION);
        }
    }
}
