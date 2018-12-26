
package org.elasticsearch.search.fetch.subphase;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.index.NumericDocValues;
import org.apache.lucene.index.ReaderUtil;
import org.elasticsearch.common.lucene.uid.Versions;
import org.elasticsearch.index.mapper.VersionFieldMapper;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.fetch.FetchSubPhase;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.Arrays;
import java.util.Comparator;

public final class VersionFetchSubPhase implements FetchSubPhase {
    @Override
    public void hitsExecute(SearchContext context, SearchHit[] hits) throws IOException {
        if (context.version() == false ||
            (context.storedFieldsContext() != null && context.storedFieldsContext().fetchFields() == false)) {
            return;
        }

        hits = hits.clone();         Arrays.sort(hits, Comparator.comparingInt(SearchHit::docId));

        int lastReaderId = -1;
        NumericDocValues versions = null;
        for (SearchHit hit : hits) {
            int readerId = ReaderUtil.subIndex(hit.docId(), context.searcher().getIndexReader().leaves());
            LeafReaderContext subReaderContext = context.searcher().getIndexReader().leaves().get(readerId);
            if (lastReaderId != readerId) {
                versions = subReaderContext.reader().getNumericDocValues(VersionFieldMapper.NAME);
                lastReaderId = readerId;
            }
            int docId = hit.docId() - subReaderContext.docBase;
            long version = Versions.NOT_FOUND;
            if (versions != null && versions.advanceExact(docId)) {
                version = versions.longValue();
            }
            hit.version(version < 0 ? -1 : version);
        }
    }
}
