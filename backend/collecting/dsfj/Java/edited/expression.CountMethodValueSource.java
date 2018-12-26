

package org.elasticsearch.script.expression;

import java.io.IOException;
import java.util.Map;
import java.util.Objects;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.ValueSource;
import org.apache.lucene.queries.function.docvalues.DoubleDocValues;
import org.elasticsearch.index.fielddata.AtomicNumericFieldData;
import org.elasticsearch.index.fielddata.IndexFieldData;
import org.elasticsearch.index.fielddata.SortedNumericDoubleValues;


final class CountMethodValueSource extends ValueSource {
    IndexFieldData<?> fieldData;

    CountMethodValueSource(IndexFieldData<?> fieldData) {
        Objects.requireNonNull(fieldData);

        this.fieldData = fieldData;
    }

    @Override
    @SuppressWarnings("rawtypes")     public FunctionValues getValues(Map context, LeafReaderContext leaf) throws IOException {
        AtomicNumericFieldData leafData = (AtomicNumericFieldData) fieldData.load(leaf);
        final SortedNumericDoubleValues values = leafData.getDoubleValues();

        return new DoubleDocValues(this) {
            @Override
            public double doubleVal(int doc) throws IOException {
                if (values.advanceExact(doc)) {
                    return values.docValueCount();
                } else {
                    return 0;
                }
            }
        };
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        FieldDataValueSource that = (FieldDataValueSource) o;

        return fieldData.equals(that.fieldData);
    }

    @Override
    public int hashCode() {
        return 31 * getClass().hashCode() + fieldData.hashCode();
    }

    @Override
    public String description() {
        return "count: field(" + fieldData.getFieldName() + ")";
    }
}
