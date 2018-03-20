

package org.elasticsearch.search;

import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.indices.IndicesService;
import org.elasticsearch.node.MockNode;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.script.ScriptService;
import org.elasticsearch.search.fetch.FetchPhase;
import org.elasticsearch.search.internal.SearchContext;
import org.elasticsearch.threadpool.ThreadPool;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class MockSearchService extends SearchService {
    
    public static class TestPlugin extends Plugin {}

    private static final Map<SearchContext, Throwable> ACTIVE_SEARCH_CONTEXTS = new ConcurrentHashMap<>();

    
    public static void assertNoInFlightContext() {
        final Map<SearchContext, Throwable> copy = new HashMap<>(ACTIVE_SEARCH_CONTEXTS);
        if (copy.isEmpty() == false) {
            throw new AssertionError(
                    "There are still [" + copy.size()
                            + "] in-flight contexts. The first one's creation site is listed as the cause of this exception.",
                    copy.values().iterator().next());
        }
    }

    
    static void addActiveContext(SearchContext context) {
        ACTIVE_SEARCH_CONTEXTS.put(context, new RuntimeException(context.toString()));
    }

    
    static void removeActiveContext(SearchContext context) {
        ACTIVE_SEARCH_CONTEXTS.remove(context);
    }

    public MockSearchService(ClusterService clusterService,
            IndicesService indicesService, ThreadPool threadPool, ScriptService scriptService,
            BigArrays bigArrays, FetchPhase fetchPhase) {
        super(clusterService, indicesService, threadPool, scriptService, bigArrays, fetchPhase, null);
    }

    @Override
    protected void putContext(SearchContext context) {
        super.putContext(context);
        addActiveContext(context);
    }

    @Override
    protected SearchContext removeContext(long id) {
        final SearchContext removed = super.removeContext(id);
        if (removed != null) {
            removeActiveContext(removed);
        }
        return removed;
    }
}
