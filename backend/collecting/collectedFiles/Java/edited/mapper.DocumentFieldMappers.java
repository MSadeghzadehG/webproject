

package org.elasticsearch.index.mapper;

import org.apache.lucene.analysis.Analyzer;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.index.analysis.FieldNameAnalyzer;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public final class DocumentFieldMappers implements Iterable<FieldMapper> {

    
    private final Map<String, FieldMapper> fieldMappers;

    private final FieldNameAnalyzer indexAnalyzer;
    private final FieldNameAnalyzer searchAnalyzer;
    private final FieldNameAnalyzer searchQuoteAnalyzer;

    private static void put(Map<String, Analyzer> analyzers, String key, Analyzer value, Analyzer defaultValue) {
        if (value == null) {
            value = defaultValue;
        }
        analyzers.put(key, value);
    }

    public DocumentFieldMappers(Collection<FieldMapper> mappers, Analyzer defaultIndex, Analyzer defaultSearch, Analyzer defaultSearchQuote) {
        Map<String, FieldMapper> fieldMappers = new HashMap<>();
        Map<String, Analyzer> indexAnalyzers = new HashMap<>();
        Map<String, Analyzer> searchAnalyzers = new HashMap<>();
        Map<String, Analyzer> searchQuoteAnalyzers = new HashMap<>();
        for (FieldMapper mapper : mappers) {
            fieldMappers.put(mapper.name(), mapper);
            MappedFieldType fieldType = mapper.fieldType();
            put(indexAnalyzers, fieldType.name(), fieldType.indexAnalyzer(), defaultIndex);
            put(searchAnalyzers, fieldType.name(), fieldType.searchAnalyzer(), defaultSearch);
            put(searchQuoteAnalyzers, fieldType.name(), fieldType.searchQuoteAnalyzer(), defaultSearchQuote);
        }
        this.fieldMappers = Collections.unmodifiableMap(fieldMappers);
        this.indexAnalyzer = new FieldNameAnalyzer(indexAnalyzers);
        this.searchAnalyzer = new FieldNameAnalyzer(searchAnalyzers);
        this.searchQuoteAnalyzer = new FieldNameAnalyzer(searchQuoteAnalyzers);
    }

    
    public FieldMapper getMapper(String field) {
        return fieldMappers.get(field);
    }

    public Collection<String> simpleMatchToFullName(String pattern) {
        Set<String> fields = new HashSet<>();
        for (FieldMapper fieldMapper : this) {
            if (Regex.simpleMatch(pattern, fieldMapper.fieldType().name())) {
                fields.add(fieldMapper.fieldType().name());
            }
        }
        return fields;
    }

    public FieldMapper smartNameFieldMapper(String name) {
        FieldMapper fieldMapper = getMapper(name);
        if (fieldMapper != null) {
            return fieldMapper;
        }
        for (FieldMapper otherFieldMapper : this) {
            if (otherFieldMapper.fieldType().name().equals(name)) {
                return otherFieldMapper;
            }
        }
        return null;
    }

    
    public Analyzer indexAnalyzer() {
        return this.indexAnalyzer;
    }

    
    public Analyzer searchAnalyzer() {
        return this.searchAnalyzer;
    }

    public Analyzer searchQuoteAnalyzer() {
        return this.searchQuoteAnalyzer;
    }

    public Iterator<FieldMapper> iterator() {
        return fieldMappers.values().iterator();
    }
}
