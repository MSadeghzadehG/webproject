
package org.elasticsearch.search.fetch;

import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.search.IndexSearcher;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public interface FetchSubPhase {

    class HitContext {
        private SearchHit hit;
        private IndexSearcher searcher;
        private LeafReaderContext readerContext;
        private int docId;
        private Map<String, Object> cache;

        public void reset(SearchHit hit, LeafReaderContext context, int docId, IndexSearcher searcher) {
            this.hit = hit;
            this.readerContext = context;
            this.docId = docId;
            this.searcher = searcher;
        }

        public SearchHit hit() {
            return hit;
        }

        public LeafReader reader() {
            return readerContext.reader();
        }

        public LeafReaderContext readerContext() {
            return readerContext;
        }

        public int docId() {
            return docId;
        }

        public IndexReader topLevelReader() {
            return searcher.getIndexReader();
        }

        public Map<String, Object> cache() {
            if (cache == null) {
                cache = new HashMap<>();
            }
            return cache;
        }

    }

    
    default void hitExecute(SearchContext context, HitContext hitContext) throws IOException {}


    default void hitsExecute(SearchContext context, SearchHit[] hits) throws IOException {}
}
