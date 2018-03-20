
package org.elasticsearch.search.aggregations.support.values;

import org.apache.lucene.search.Scorer;
import org.elasticsearch.common.lucene.ScorerAware;
import org.elasticsearch.index.fielddata.SortingNumericDoubleValues;
import org.elasticsearch.script.SearchScript;
import org.elasticsearch.search.aggregations.AggregationExecutionException;
import org.joda.time.ReadableInstant;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Collection;


public class ScriptDoubleValues extends SortingNumericDoubleValues implements ScorerAware {

    final SearchScript script;

    public ScriptDoubleValues(SearchScript script) {
        super();
        this.script = script;
    }

    @Override
    public boolean advanceExact(int target) throws IOException {
        script.setDocument(target);
        final Object value = script.run();

        if (value == null) {
            return false;
        } else if (value instanceof Number) {
            resize(1);
            values[0] = ((Number) value).doubleValue();
        } else if (value instanceof ReadableInstant) {
            resize(1);
            values[0] = ((ReadableInstant) value).getMillis();
        } else if (value.getClass().isArray()) {
            int length = Array.getLength(value);
            if (length == 0) {
                return false;
            }
            resize(length);
            for (int i = 0; i < length; ++i) {
                values[i] = toDoubleValue(Array.get(value, i));
            }
        } else if (value instanceof Collection) {
            Collection<?> coll = (Collection<?>) value;
            if (coll.isEmpty()) {
                return false;
            }
            resize(coll.size());
            int i = 0;
            for (Object v : coll) {
                values[i++] = toDoubleValue(v);
            }
            assert i == docValueCount();
        } else {
            resize(1);
            values[0] = toDoubleValue(value);
        }

        sort();
        return true;
    }

    private static double toDoubleValue(Object o) {
        if (o instanceof Number) {
            return ((Number) o).doubleValue();
        } else if (o instanceof ReadableInstant) {
                        return ((ReadableInstant) o).getMillis();
        } else if (o instanceof Boolean) {
                                                            return ((Boolean) o).booleanValue() ? 1.0 : 0.0;
        } else {
            throw new AggregationExecutionException("Unsupported script value [" + o + "], expected a number, date, or boolean");
        }
    }

    @Override
    public void setScorer(Scorer scorer) {
        script.setScorer(scorer);
    }
}
