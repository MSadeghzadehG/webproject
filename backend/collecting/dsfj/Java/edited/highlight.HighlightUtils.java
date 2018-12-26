
package org.elasticsearch.search.fetch.subphase.highlight;

import org.apache.lucene.search.highlight.DefaultEncoder;
import org.apache.lucene.search.highlight.Encoder;
import org.apache.lucene.search.highlight.SimpleHTMLEncoder;
import org.elasticsearch.index.fieldvisitor.CustomFieldsVisitor;
import org.elasticsearch.index.mapper.FieldMapper;
import org.elasticsearch.search.fetch.FetchSubPhase;
import org.elasticsearch.search.internal.SearchContext;
import org.elasticsearch.search.lookup.SourceLookup;

import java.io.IOException;
import java.util.Collections;
import java.util.List;

import static java.util.Collections.singleton;

public final class HighlightUtils {

        public static final char PARAGRAPH_SEPARATOR = 8233;
    public static final char NULL_SEPARATOR = '\u0000';

    private HighlightUtils() {

    }

    
    public static List<Object> loadFieldValues(SearchContextHighlight.Field field, FieldMapper mapper, SearchContext searchContext,
            FetchSubPhase.HitContext hitContext) throws IOException {
                boolean forceSource = searchContext.highlight().forceSource(field);
        List<Object> textsToHighlight;
        if (!forceSource && mapper.fieldType().stored()) {
            CustomFieldsVisitor fieldVisitor = new CustomFieldsVisitor(singleton(mapper.fieldType().name()), false);
            hitContext.reader().document(hitContext.docId(), fieldVisitor);
            textsToHighlight = fieldVisitor.fields().get(mapper.fieldType().name());
            if (textsToHighlight == null) {
                                textsToHighlight = Collections.emptyList();
            }
        } else {
            SourceLookup sourceLookup = searchContext.lookup().source();
            sourceLookup.setSegmentAndDocument(hitContext.readerContext(), hitContext.docId());
            textsToHighlight = sourceLookup.extractRawValues(mapper.fieldType().name());
        }
        assert textsToHighlight != null;
        return textsToHighlight;
    }

    static class Encoders {
        static final Encoder DEFAULT = new DefaultEncoder();
        static final Encoder HTML = new SimpleHTMLEncoder();
    }
}
