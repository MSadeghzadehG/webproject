

package org.elasticsearch.plugins;

import org.elasticsearch.index.mapper.Mapper;
import org.elasticsearch.index.mapper.MetadataFieldMapper;

import java.util.Collections;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Predicate;


public interface MapperPlugin {

    
    default Map<String, Mapper.TypeParser> getMappers() {
        return Collections.emptyMap();
    }

    
    default Map<String, MetadataFieldMapper.TypeParser> getMetadataMappers() {
        return Collections.emptyMap();
    }

    
    default Function<String, Predicate<String>> getFieldFilter() {
        return NOOP_FIELD_FILTER;
    }

    
    Predicate<String> NOOP_FIELD_PREDICATE = field -> true;

    
    Function<String, Predicate<String>> NOOP_FIELD_FILTER = index -> NOOP_FIELD_PREDICATE;
}
