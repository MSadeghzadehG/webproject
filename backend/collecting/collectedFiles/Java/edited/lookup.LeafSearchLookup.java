

package org.elasticsearch.search.lookup;

import org.apache.lucene.index.LeafReaderContext;

import java.util.HashMap;
import java.util.Map;

import static java.util.Collections.unmodifiableMap;


public class LeafSearchLookup {

    final LeafReaderContext ctx;
    final LeafDocLookup docMap;
    final SourceLookup sourceLookup;
    final LeafFieldsLookup fieldsLookup;
    final Map<String, Object> asMap;

    public LeafSearchLookup(LeafReaderContext ctx, LeafDocLookup docMap, SourceLookup sourceLookup,
            LeafFieldsLookup fieldsLookup) {
        this.ctx = ctx;
        this.docMap = docMap;
        this.sourceLookup = sourceLookup;
        this.fieldsLookup = fieldsLookup;

        Map<String, Object> asMap = new HashMap<>(4);
        asMap.put("doc", docMap);
        asMap.put("_doc", docMap);
        asMap.put("_source", sourceLookup);
        asMap.put("_fields", fieldsLookup);
        this.asMap = unmodifiableMap(asMap);
    }

    public Map<String, Object> asMap() {
        return this.asMap;
    }

    public SourceLookup source() {
        return this.sourceLookup;
    }

    public LeafFieldsLookup fields() {
        return this.fieldsLookup;
    }

    public LeafDocLookup doc() {
        return this.docMap;
    }

    public void setDocument(int docId) {
        docMap.setDocument(docId);
        sourceLookup.setSegmentAndDocument(ctx, docId);
        fieldsLookup.setDocument(docId);
    }
}
