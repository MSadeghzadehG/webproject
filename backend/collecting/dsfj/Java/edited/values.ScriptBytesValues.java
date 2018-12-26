
package org.elasticsearch.search.aggregations.support.values;

import org.apache.lucene.search.Scorer;
import org.elasticsearch.common.lucene.ScorerAware;
import org.elasticsearch.common.util.CollectionUtils;
import org.elasticsearch.index.fielddata.SortedBinaryDocValues;
import org.elasticsearch.index.fielddata.SortingBinaryDocValues;
import org.elasticsearch.script.SearchScript;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Collection;


public class ScriptBytesValues extends SortingBinaryDocValues implements ScorerAware {

    private final SearchScript script;

    public ScriptBytesValues(SearchScript script) {
        super();
        this.script = script;
    }

    private void set(int i, Object o) {
        if (o == null) {
            values[i].clear();
        } else {
            CollectionUtils.ensureNoSelfReferences(o);
            values[i].copyChars(o.toString());
        }
    }

    @Override
    public boolean advanceExact(int doc) throws IOException {
        script.setDocument(doc);
        final Object value = script.run();
        if (value == null) {
            return false;
        } else if (value.getClass().isArray()) {
            count = Array.getLength(value);
            if (count == 0) {
                return false;
            }
            grow();
            for (int i = 0; i < count; ++i) {
                set(i, Array.get(value, i));
            }
        } else if (value instanceof Collection) {
            final Collection<?> coll = (Collection<?>) value;
            count = coll.size();
            if (count == 0) {
                return false;
            }
            grow();
            int i = 0;
            for (Object v : coll) {
                set(i++, v);
            }
        } else {
            count = 1;
            set(0, value);
        }
        sort();
        return true;
    }

    @Override
    public void setScorer(Scorer scorer) {
        script.setScorer(scorer);
    }
}
