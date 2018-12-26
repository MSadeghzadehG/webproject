

package org.elasticsearch.index.engine;

import org.apache.logging.log4j.Logger;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.ReferenceManager;
import org.apache.lucene.search.SearcherManager;
import org.apache.lucene.store.AlreadyClosedException;
import org.elasticsearch.index.store.Store;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;


final class EngineSearcher extends Engine.Searcher {
    private final AtomicBoolean released = new AtomicBoolean(false);
    private final Store store;
    private final Logger logger;
    private final ReferenceManager<IndexSearcher> referenceManager;

    EngineSearcher(String source, ReferenceManager<IndexSearcher> searcherReferenceManager, Store store, Logger logger) throws IOException {
        super(source, searcherReferenceManager.acquire());
        this.store = store;
        this.logger = logger;
        this.referenceManager = searcherReferenceManager;
    }

    @Override
    public void close() {
        if (!released.compareAndSet(false, true)) {
                
            logger.warn("Searcher was released twice", new IllegalStateException("Double release"));
            return;
        }
        try {
            referenceManager.release(searcher());
        } catch (IOException e) {
            throw new IllegalStateException("Cannot close", e);
        } catch (AlreadyClosedException e) {
                        throw new AssertionError(e);
        } finally {
            store.decRef();
        }
    }
}
