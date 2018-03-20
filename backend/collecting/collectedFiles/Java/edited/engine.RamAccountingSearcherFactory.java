

package org.elasticsearch.index.engine;

import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.index.SegmentReader;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.SearcherFactory;
import org.elasticsearch.common.breaker.CircuitBreaker;
import org.elasticsearch.common.lucene.Lucene;
import org.elasticsearch.indices.breaker.CircuitBreakerService;

import java.io.IOException;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


final class RamAccountingSearcherFactory extends SearcherFactory {

    private final CircuitBreakerService breakerService;

    RamAccountingSearcherFactory(CircuitBreakerService breakerService) {
        this.breakerService = breakerService;
    }

    @Override
    public IndexSearcher newSearcher(IndexReader reader, IndexReader previousReader) throws IOException {
        final CircuitBreaker breaker = breakerService.getBreaker(CircuitBreaker.ACCOUNTING);

                                                final Set<IndexReader.CacheKey> prevReaders;
        if (previousReader == null) {
            prevReaders = Collections.emptySet();
        } else {
            final List<LeafReaderContext> previousReaderLeaves = previousReader.leaves();
            prevReaders = new HashSet<>(previousReaderLeaves.size());
            for (LeafReaderContext lrc : previousReaderLeaves) {
                prevReaders.add(Lucene.segmentReader(lrc.reader()).getCoreCacheHelper().getKey());
            }
        }

        for (LeafReaderContext lrc : reader.leaves()) {
            final SegmentReader segmentReader = Lucene.segmentReader(lrc.reader());
                                    if (prevReaders.contains(segmentReader.getCoreCacheHelper().getKey()) == false) {
                final long ramBytesUsed = segmentReader.ramBytesUsed();
                                breaker.addWithoutBreaking(ramBytesUsed);
                                                segmentReader.getCoreCacheHelper().addClosedListener(k -> breaker.addWithoutBreaking(-ramBytesUsed));
            }
        }
        return super.newSearcher(reader, previousReader);
    }
}
