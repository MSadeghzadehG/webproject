

package org.elasticsearch.indices.mapper;

import org.elasticsearch.index.mapper.Mapper;
import org.elasticsearch.index.mapper.MetadataFieldMapper;
import org.elasticsearch.plugins.MapperPlugin;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Predicate;


public final class MapperRegistry {

    private final Map<String, Mapper.TypeParser> mapperParsers;
    private final Map<String, MetadataFieldMapper.TypeParser> metadataMapperParsers;
    private final Function<String, Predicate<String>> fieldFilter;


    public MapperRegistry(Map<String, Mapper.TypeParser> mapperParsers,
            Map<String, MetadataFieldMapper.TypeParser> metadataMapperParsers, Function<String, Predicate<String>> fieldFilter) {
        this.mapperParsers = Collections.unmodifiableMap(new LinkedHashMap<>(mapperParsers));
        this.metadataMapperParsers = Collections.unmodifiableMap(new LinkedHashMap<>(metadataMapperParsers));
        this.fieldFilter = fieldFilter;
    }

    
    public Map<String, Mapper.TypeParser> getMapperParsers() {
        return mapperParsers;
    }

    
    public Map<String, MetadataFieldMapper.TypeParser> getMetadataMapperParsers() {
        return metadataMapperParsers;
    }

    
    public boolean isMetaDataField(String field) {
        return getMetadataMapperParsers().containsKey(field);
    }

    
    public Function<String, Predicate<String>> getFieldFilter() {
        return fieldFilter;
    }
}
