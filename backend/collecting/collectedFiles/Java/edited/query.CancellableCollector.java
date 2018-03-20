
package org.elasticsearch.search.query;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.search.Collector;
import org.apache.lucene.search.FilterCollector;
import org.apache.lucene.search.LeafCollector;
import org.elasticsearch.tasks.TaskCancelledException;

import java.io.IOException;
import java.util.function.BooleanSupplier;


public class CancellableCollector extends FilterCollector {
    private final BooleanSupplier cancelled;

    
    public CancellableCollector(BooleanSupplier cancelled, Collector in) {
        super(in);
        this.cancelled = cancelled;
    }

    @Override
    public LeafCollector getLeafCollector(LeafReaderContext context) throws IOException {
        if (cancelled.getAsBoolean()) {
            throw new TaskCancelledException("cancelled");
        }
        return super.getLeafCollector(context);
    }
}
