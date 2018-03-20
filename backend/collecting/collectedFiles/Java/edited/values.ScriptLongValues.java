
package org.elasticsearch.search.aggregations.support.values;

import org.apache.lucene.search.Scorer;
import org.apache.lucene.util.LongValues;
import org.elasticsearch.common.lucene.ScorerAware;
import org.elasticsearch.index.fielddata.AbstractSortingNumericDocValues;
import org.elasticsearch.script.SearchScript;
import org.elasticsearch.search.aggregations.AggregationExecutionException;
import org.joda.time.ReadableInstant;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Collection;
import java.util.Iterator;


public class ScriptLongValues extends AbstractSortingNumericDocValues implements ScorerAware {

    final SearchScript script;

    public ScriptLongValues(SearchScript script) {
        super();
        this.script = script;
    }

    @Override
    public boolean advanceExact(int target) throws IOException {
        script.setDocument(target);
        final Object value = script.run();

        if (value == null) {
            return false;
        }

        else if (value.getClass().isArray()) {
            int length = Array.getLength(value);
            if (length == 0) {
                return false;
            }
            resize(length);
            for (int i = 0; i < length; ++i) {
                values[i] = toLongValue(Array.get(value, i));
            }
        }

        else if (value instanceof Collection) {
            Collection<?> coll = (Collection<?>) value;
            if (coll.isEmpty()) {
                return false;
            }
            resize(coll.size());
            int i = 0;
            for (Iterator<?> it = coll.iterator(); it.hasNext(); ++i) {
                values[i] = toLongValue(it.next());
            }
            assert i == docValueCount();
        }

        else {
            resize(1);
            values[0] = toLongValue(value);
        }

        sort();
        return true;
    }

    private static long toLongValue(Object o) {
        if (o instanceof Number) {
            return ((Number) o).longValue();
        } else if (o instanceof ReadableInstant) {
                        return ((ReadableInstant) o).getMillis();
        } else if (o instanceof Boolean) {
                                                            return ((Boolean) o).booleanValue() ? 1L : 0L;
        } else {
            throw new AggregationExecutionException("Unsupported script value [" + o + "], expected a number, date, or boolean");
        }
    }

    @Override
    public void setScorer(Scorer scorer) {
        script.setScorer(scorer);
    }
}
