

package org.elasticsearch.script.expression;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.ValueSource;
import org.apache.lucene.queries.function.docvalues.DoubleDocValues;
import org.elasticsearch.index.fielddata.AtomicNumericFieldData;
import org.elasticsearch.index.fielddata.IndexFieldData;
import org.elasticsearch.index.fielddata.SortedNumericDoubleValues;

import java.io.IOException;
import java.util.Map;
import java.util.Objects;


final class EmptyMemberValueSource extends ValueSource {
    final IndexFieldData<?> fieldData;

    EmptyMemberValueSource(IndexFieldData<?> fieldData) {
        this.fieldData = Objects.requireNonNull(fieldData);
    }

    @Override
    @SuppressWarnings("rawtypes")     public FunctionValues getValues(Map context, LeafReaderContext leaf) throws IOException {
        AtomicNumericFieldData leafData = (AtomicNumericFieldData) fieldData.load(leaf);
        final SortedNumericDoubleValues values = leafData.getDoubleValues();
        return new DoubleDocValues(this) {
            @Override
            public double doubleVal(int doc) throws IOException {
                if (values.advanceExact(doc)) {
                    return 0;
                } else {
                    return 1;
                }
            }
        };
    }

    @Override
    public int hashCode() {
        return 31 * getClass().hashCode() + fieldData.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null) return false;
        if (getClass() != obj.getClass()) return false;
        EmptyMemberValueSource other = (EmptyMemberValueSource) obj;
        if (!fieldData.equals(other.fieldData)) return false;
        return true;
    }

    @Override
    public String description() {
        return "empty: field(" + fieldData.getFieldName() + ")";
    }
}
