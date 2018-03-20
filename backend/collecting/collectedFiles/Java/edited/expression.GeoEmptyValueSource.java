

package org.elasticsearch.script.expression;

import java.io.IOException;
import java.util.Map;
import java.util.Objects;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.ValueSource;
import org.apache.lucene.queries.function.docvalues.DoubleDocValues;
import org.elasticsearch.index.fielddata.AtomicGeoPointFieldData;
import org.elasticsearch.index.fielddata.IndexFieldData;
import org.elasticsearch.index.fielddata.MultiGeoPointValues;


final class GeoEmptyValueSource extends ValueSource {
    IndexFieldData<?> fieldData;

    GeoEmptyValueSource(IndexFieldData<?> fieldData) {
        this.fieldData = Objects.requireNonNull(fieldData);
    }

    @Override
    @SuppressWarnings("rawtypes")     public FunctionValues getValues(Map context, LeafReaderContext leaf) throws IOException {
        AtomicGeoPointFieldData leafData = (AtomicGeoPointFieldData) fieldData.load(leaf);
        final MultiGeoPointValues values = leafData.getGeoPointValues();
        return new DoubleDocValues(this) {
            @Override
            public double doubleVal(int doc) throws IOException {
                if (values.advanceExact(doc)) {
                    return 1;
                } else {
                    return 0;
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
        GeoEmptyValueSource other = (GeoEmptyValueSource) obj;
        if (!fieldData.equals(other.fieldData)) return false;
        return true;
    }

    @Override
    public String description() {
        return "empty: field(" + fieldData.getFieldName() + ")";
    }
}
