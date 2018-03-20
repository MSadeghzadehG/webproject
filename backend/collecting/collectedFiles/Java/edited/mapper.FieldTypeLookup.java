

package org.elasticsearch.index.mapper;

import org.elasticsearch.common.collect.CopyOnWriteHashMap;
import org.elasticsearch.common.regex.Regex;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;


class FieldTypeLookup implements Iterable<MappedFieldType> {

    
    final CopyOnWriteHashMap<String, MappedFieldType> fullNameToFieldType;

    
    FieldTypeLookup() {
        fullNameToFieldType = new CopyOnWriteHashMap<>();
    }

    private FieldTypeLookup(CopyOnWriteHashMap<String, MappedFieldType> fullName) {
        this.fullNameToFieldType = fullName;
    }

    
    public FieldTypeLookup copyAndAddAll(String type, Collection<FieldMapper> fieldMappers) {
        Objects.requireNonNull(type, "type must not be null");
        if (MapperService.DEFAULT_MAPPING.equals(type)) {
            throw new IllegalArgumentException("Default mappings should not be added to the lookup");
        }

        CopyOnWriteHashMap<String, MappedFieldType> fullName = this.fullNameToFieldType;

        for (FieldMapper fieldMapper : fieldMappers) {
            MappedFieldType fieldType = fieldMapper.fieldType();
            MappedFieldType fullNameFieldType = fullName.get(fieldType.name());

            if (fullNameFieldType == null) {
                                fullName = fullName.copyAndPut(fieldType.name(), fieldMapper.fieldType());
            } else {
                                checkCompatibility(fullNameFieldType, fieldType);
                if (fieldType.equals(fullNameFieldType) == false) {
                    fullName = fullName.copyAndPut(fieldType.name(), fieldMapper.fieldType());
                }
            }
        }
        return new FieldTypeLookup(fullName);
    }

    
    private void checkCompatibility(MappedFieldType existingFieldType, MappedFieldType newFieldType) {
        List<String> conflicts = new ArrayList<>();
        existingFieldType.checkCompatibility(newFieldType, conflicts);
        if (conflicts.isEmpty() == false) {
            throw new IllegalArgumentException("Mapper for [" + newFieldType.name() + "] conflicts with existing mapping:\n" + conflicts.toString());
        }
    }

    
    public MappedFieldType get(String field) {
        return fullNameToFieldType.get(field);
    }

    
    public Collection<String> simpleMatchToFullName(String pattern) {
        Set<String> fields = new HashSet<>();
        for (MappedFieldType fieldType : this) {
            if (Regex.simpleMatch(pattern, fieldType.name())) {
                fields.add(fieldType.name());
            }
        }
        return fields;
    }

    @Override
    public Iterator<MappedFieldType> iterator() {
        return fullNameToFieldType.values().iterator();
    }
}
