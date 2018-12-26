

package org.elasticsearch.search.aggregations;


import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.search.Collector;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.StreamSupport;


public abstract class BucketCollector implements Collector {

    public static final BucketCollector NO_OP_COLLECTOR = new BucketCollector() {

        @Override
        public LeafBucketCollector getLeafCollector(LeafReaderContext reader) {
            return LeafBucketCollector.NO_OP_COLLECTOR;
        }
        @Override
        public void preCollection() throws IOException {
                    }
        @Override
        public void postCollection() throws IOException {
                    }
        @Override
        public boolean needsScores() {
            return false;
        }
    };

    
    public static BucketCollector wrap(Iterable<? extends BucketCollector> collectorList) {
        final BucketCollector[] collectors =
                StreamSupport.stream(collectorList.spliterator(), false).toArray(size -> new BucketCollector[size]);
        switch (collectors.length) {
            case 0:
                return NO_OP_COLLECTOR;
            case 1:
                return collectors[0];
            default:
                return new BucketCollector() {

                    @Override
                    public LeafBucketCollector getLeafCollector(LeafReaderContext ctx) throws IOException {
                        List<LeafBucketCollector> leafCollectors = new ArrayList<>(collectors.length);
                        for (BucketCollector c : collectors) {
                            leafCollectors.add(c.getLeafCollector(ctx));
                        }
                        return LeafBucketCollector.wrap(leafCollectors);
                    }

                    @Override
                    public void preCollection() throws IOException {
                        for (BucketCollector collector : collectors) {
                            collector.preCollection();
                        }
                    }

                    @Override
                    public void postCollection() throws IOException {
                        for (BucketCollector collector : collectors) {
                            collector.postCollection();
                        }
                    }

                    @Override
                    public boolean needsScores() {
                        for (BucketCollector collector : collectors) {
                            if (collector.needsScores()) {
                                return true;
                            }
                        }
                        return false;
                    }

                    @Override
                    public String toString() {
                        return Arrays.toString(collectors);
                    }
                };
        }
    }

    @Override
    public abstract LeafBucketCollector getLeafCollector(LeafReaderContext ctx) throws IOException;

    
    public abstract void preCollection() throws IOException;

    
    public abstract void postCollection() throws IOException;

}
