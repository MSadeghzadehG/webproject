

package org.elasticsearch.search.internal;

import org.apache.lucene.search.ScoreDoc;
import org.elasticsearch.search.Scroll;

import java.util.HashMap;
import java.util.Map;


public final class ScrollContext {

    private Map<String, Object> context = null;

    public long totalHits = -1;
    public float maxScore;
    public ScoreDoc lastEmittedDoc;
    public Scroll scroll;

    
    @SuppressWarnings("unchecked")     public <T> T getFromContext(String key) {
        return context != null ? (T) context.get(key) : null;
    }

    
    public void putInContext(String key, Object value) {
        if (context == null) {
            context = new HashMap<>();
        }
        context.put(key, value);
    }
}
