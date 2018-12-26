

package org.elasticsearch.search.profile;

import org.elasticsearch.search.internal.ContextIndexSearcher;
import org.elasticsearch.search.profile.aggregation.AggregationProfiler;
import org.elasticsearch.search.profile.query.QueryProfiler;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public final class Profilers {

    private final ContextIndexSearcher searcher;
    private final List<QueryProfiler> queryProfilers;
    private final AggregationProfiler aggProfiler;

    
    public Profilers(ContextIndexSearcher searcher) {
        this.searcher = searcher;
        this.queryProfilers = new ArrayList<>();
        this.aggProfiler = new AggregationProfiler();
        addQueryProfiler();
    }

    
    public QueryProfiler addQueryProfiler() {
        QueryProfiler profiler = new QueryProfiler();
        searcher.setProfiler(profiler);
        queryProfilers.add(profiler);
        return profiler;
    }

    
    public QueryProfiler getCurrentQueryProfiler() {
        return queryProfilers.get(queryProfilers.size() - 1);
    }

    
    public List<QueryProfiler> getQueryProfilers() {
        return Collections.unmodifiableList(queryProfilers);
    }

    
    public AggregationProfiler getAggregationProfiler() {
        return aggProfiler;
    }

}
