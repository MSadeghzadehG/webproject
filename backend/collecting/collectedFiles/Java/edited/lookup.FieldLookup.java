
package org.elasticsearch.search.lookup;

import org.elasticsearch.index.mapper.MappedFieldType;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class FieldLookup {

        private final MappedFieldType fieldType;

    private Map<String, List<Object>> fields;

    private Object value;

    private boolean valueLoaded = false;

    private List<Object> values = new ArrayList<>();

    private boolean valuesLoaded = false;

    FieldLookup(MappedFieldType fieldType) {
        this.fieldType = fieldType;
    }

    public MappedFieldType fieldType() {
        return fieldType;
    }

    public Map<String, List<Object>> fields() {
        return fields;
    }

    
    public void fields(Map<String, List<Object>> fields) {
        this.fields = fields;
    }

    public void clear() {
        value = null;
        valueLoaded = false;
        values.clear();
        valuesLoaded = false;
        fields = null;
    }

    public boolean isEmpty() {
        if (valueLoaded) {
            return value == null;
        }
        if (valuesLoaded) {
            return values.isEmpty();
        }
        return getValue() == null;
    }

    public Object getValue() {
        if (valueLoaded) {
            return value;
        }
        valueLoaded = true;
        value = null;
        List<Object> values = fields.get(fieldType.name());
        return values != null ? value = values.get(0) : null;
    }

    public List<Object> getValues() {
        if (valuesLoaded) {
            return values;
        }
        valuesLoaded = true;
        values.clear();
        return values = fields().get(fieldType.name());
    }
}
