
package org.elasticsearch.search.fetch.subphase.highlight;

import org.elasticsearch.common.text.Text;
import org.elasticsearch.index.mapper.FieldMapper;
import org.elasticsearch.search.fetch.subphase.highlight.HighlightField;
import org.elasticsearch.search.fetch.subphase.highlight.Highlighter;
import org.elasticsearch.search.fetch.subphase.highlight.HighlighterContext;
import org.elasticsearch.search.fetch.subphase.highlight.SearchContextHighlight;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;


public class CustomHighlighter implements Highlighter {

    @Override
    public HighlightField highlight(HighlighterContext highlighterContext) {
        SearchContextHighlight.Field field = highlighterContext.field;
        CacheEntry cacheEntry = (CacheEntry) highlighterContext.hitContext.cache().get("test-custom");
        final int docId = highlighterContext.hitContext.readerContext().docBase + highlighterContext.hitContext.docId();
        if (cacheEntry == null) {
            cacheEntry = new CacheEntry();
            highlighterContext.hitContext.cache().put("test-custom", cacheEntry);
            cacheEntry.docId = docId;
            cacheEntry.position = 1;
        } else {
            if (cacheEntry.docId == docId) {
                cacheEntry.position++;
            } else {
                cacheEntry.docId = docId;
                cacheEntry.position = 1;
            }
        }

        List<Text> responses = new ArrayList<>();
        responses.add(new Text(String.format(Locale.ENGLISH, "standard response for %s at position %s", field.field(),
                cacheEntry.position)));

        if (field.fieldOptions().options() != null) {
            for (Map.Entry<String, Object> entry : field.fieldOptions().options().entrySet()) {
                responses.add(new Text("field:" + entry.getKey() + ":" + entry.getValue()));
            }
        }

        return new HighlightField(highlighterContext.fieldName, responses.toArray(new Text[]{}));
    }

    @Override
    public boolean canHighlight(FieldMapper fieldMapper) {
        return true;
    }

    private static class CacheEntry {
        private int position;
        private int docId;
    }
}
