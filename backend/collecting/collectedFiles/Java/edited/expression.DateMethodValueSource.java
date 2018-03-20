

package org.elasticsearch.script.expression;

import java.io.IOException;
import java.util.Calendar;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.TimeZone;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.docvalues.DoubleDocValues;
import org.elasticsearch.index.fielddata.AtomicNumericFieldData;
import org.elasticsearch.index.fielddata.IndexFieldData;
import org.elasticsearch.index.fielddata.NumericDoubleValues;
import org.elasticsearch.search.MultiValueMode;


class DateMethodValueSource extends FieldDataValueSource {

    final String methodName;
    final int calendarType;

    DateMethodValueSource(IndexFieldData<?> indexFieldData, MultiValueMode multiValueMode, String methodName, int calendarType) {
        super(indexFieldData, multiValueMode);

        Objects.requireNonNull(methodName);

        this.methodName = methodName;
        this.calendarType = calendarType;
    }

    @Override
    @SuppressWarnings("rawtypes")     public FunctionValues getValues(Map context, LeafReaderContext leaf) throws IOException {
        AtomicNumericFieldData leafData = (AtomicNumericFieldData) fieldData.load(leaf);
        final Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"), Locale.ROOT);
        NumericDoubleValues docValues = multiValueMode.select(leafData.getDoubleValues(), 0d);
        return new DoubleDocValues(this) {
            @Override
            public double doubleVal(int docId) throws IOException {
                if (docValues.advanceExact(docId)) {
                    long millis = (long)docValues.doubleValue();
                    calendar.setTimeInMillis(millis);
                    return calendar.get(calendarType);
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

        DateMethodValueSource that = (DateMethodValueSource) o;

        if (calendarType != that.calendarType) return false;
        return methodName.equals(that.methodName);

    }

    @Override
    public int hashCode() {
        int result = super.hashCode();
        result = 31 * result + methodName.hashCode();
        result = 31 * result + calendarType;
        return result;
    }
}
