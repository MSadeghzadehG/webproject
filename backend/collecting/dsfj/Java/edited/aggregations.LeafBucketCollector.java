

package org.elasticsearch.search.aggregations;

import org.apache.lucene.search.LeafCollector;
import org.apache.lucene.search.Scorer;

import java.io.IOException;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;


public abstract class LeafBucketCollector implements LeafCollector {

    public static final LeafBucketCollector NO_OP_COLLECTOR = new LeafBucketCollector() {
        @Override
        public void setScorer(Scorer arg0) throws IOException {
                    }
        @Override
        public void collect(int doc, long bucket) {
                    }
    };

    public static LeafBucketCollector wrap(Iterable<LeafBucketCollector> collectors) {
        final Stream<LeafBucketCollector> actualCollectors =
                StreamSupport.stream(collectors.spliterator(), false).filter(c -> c != NO_OP_COLLECTOR);
        final LeafBucketCollector[] colls = actualCollectors.toArray(size -> new LeafBucketCollector[size]);
        switch (colls.length) {
        case 0:
            return NO_OP_COLLECTOR;
        case 1:
            return colls[0];
        default:
            return new LeafBucketCollector() {

                @Override
                public void setScorer(Scorer s) throws IOException {
                    for (LeafBucketCollector c : colls) {
                        c.setScorer(s);
                    }
                }

                @Override
                public void collect(int doc, long bucket) throws IOException {
                    for (LeafBucketCollector c : colls) {
                        c.collect(doc, bucket);
                    }
                }

            };
        }
    }

    
    public abstract void collect(int doc, long bucket) throws IOException;

    @Override
    public final void collect(int doc) throws IOException {
        collect(doc, 0);
    }

    @Override
    public void setScorer(Scorer scorer) throws IOException {
            }
}
