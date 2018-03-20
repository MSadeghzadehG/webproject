

package org.elasticsearch.script.expression;

import java.io.IOException;
import java.util.Map;
import java.util.Objects;
import java.util.function.ToIntFunction;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.docvalues.DoubleDocValues;
import org.elasticsearch.index.fielddata.AtomicNumericFieldData;
import org.elasticsearch.index.fielddata.IndexFieldData;
import org.elasticsearch.index.fielddata.NumericDoubleValues;
import org.elasticsearch.search.MultiValueMode;
import org.joda.time.DateTimeZone;
import org.joda.time.MutableDateTime;
import org.joda.time.ReadableDateTime;


class DateObjectValueSource extends FieldDataValueSource {

    final String methodName;
    final ToIntFunction<ReadableDateTime> function;

    DateObjectValueSource(IndexFieldData<?> indexFieldData, MultiValueMode multiValueMode, 
                          String methodName, ToIntFunction<ReadableDateTime> function) {
        super(indexFieldData, multiValueMode);

        Objects.requireNonNull(methodName);

        this.methodName = methodName;
        this.function = function;
    }

    @Override
    @SuppressWarnings("rawtypes")     public FunctionValues getValues(Map context, LeafReaderContext leaf) throws IOException {
        AtomicNumericFieldData leafData = (AtomicNumericFieldData) fieldData.load(leaf);
        MutableDateTime joda = new MutableDateTime(0, DateTimeZone.UTC);
        NumericDoubleValues docValues = multiValueMode.select(leafData.getDoubleValues(), 0d);
        return new DoubleDocValues(this) {
            @Override
            public double doubleVal(int docId) throws IOException {
                if (docValues.advanceExact(docId)) {
                    long millis = (long)docValues.doubleValue();
                    joda.setMillis(millis);
                    return function.applyAsInt(joda);
                } else {
                    return 0;
                }
            }
        };
    }

    @Override
    public String description() {
        return methodName + ": field(" + fieldData.getFieldName() + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        if (!super.equals(o)) return false;

        DateObjectValueSource that = (DateObjectValueSource) o;
        return methodName.equals(that.methodName);
    }

    @Override
    public int hashCode() {
        int result = super.hashCode();
        result = 31 * result + methodName.hashCode();
        return result;
    }
}
