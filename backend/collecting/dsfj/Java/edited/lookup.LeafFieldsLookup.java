
package org.elasticsearch.search.lookup;

import org.apache.lucene.index.LeafReader;
import org.elasticsearch.ElasticsearchParseException;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.index.fieldvisitor.SingleFieldsVisitor;
import org.elasticsearch.index.mapper.IdFieldMapper;
import org.elasticsearch.index.mapper.MappedFieldType;
import org.elasticsearch.index.mapper.MapperService;
import org.elasticsearch.index.mapper.UidFieldMapper;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import static java.util.Collections.singletonMap;

public class LeafFieldsLookup implements Map {

    private final MapperService mapperService;
    private final boolean singleType;

    @Nullable
    private final String[] types;

    private final LeafReader reader;

    private int docId = -1;

    private final Map<String, FieldLookup> cachedFieldData = new HashMap<>();

    private final SingleFieldsVisitor fieldVisitor;

    LeafFieldsLookup(MapperService mapperService, @Nullable String[] types, LeafReader reader) {
        this.mapperService = mapperService;
        this.singleType = mapperService.getIndexSettings().isSingleType();
        this.types = types;
        this.reader = reader;
        this.fieldVisitor = new SingleFieldsVisitor(null);
    }

    public void setDocument(int docId) {
        if (this.docId == docId) {             return;
        }
        this.docId = docId;
        clearCache();
    }


    @Override
    public Object get(Object key) {
        return loadFieldData(key.toString());
    }

    @Override
    public boolean containsKey(Object key) {
        try {
            loadFieldData(key.toString());
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    @Override
    public int size() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isEmpty() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set keySet() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Collection values() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set entrySet() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Object put(Object key, Object value) {
        throw new UnsupportedOperationException();
    }

    @Override
    public Object remove(Object key) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {
        throw new UnsupportedOperationException();
    }

    @Override
    public void putAll(Map m) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean containsValue(Object value) {
        throw new UnsupportedOperationException();
    }

    private FieldLookup loadFieldData(String name) {
        FieldLookup data = cachedFieldData.get(name);
        if (data == null) {
            MappedFieldType fieldType = mapperService.fullName(name);
            if (fieldType == null) {
                throw new IllegalArgumentException("No field found for [" + name + "] in mapping with types " + Arrays.toString(types));
            }
            data = new FieldLookup(fieldType);
            cachedFieldData.put(name, data);
        }
        if (data.fields() == null) {
            String fieldName = data.fieldType().name();
            if (singleType && UidFieldMapper.NAME.equals(fieldName)) {
                fieldName = IdFieldMapper.NAME;
            }
            fieldVisitor.reset(fieldName);
            try {
                reader.document(docId, fieldVisitor);
                fieldVisitor.postProcess(mapperService);
                List<Object> storedFields = fieldVisitor.fields().get(data.fieldType().name());
                data.fields(singletonMap(name, storedFields));
            } catch (IOException e) {
                throw new ElasticsearchParseException("failed to load field [{}]", e, name);
            }
        }
        return data;
    }

    private void clearCache() {
        for (Entry<String, FieldLookup> entry : cachedFieldData.entrySet()) {
            entry.getValue().clear();
        }
    }

}
