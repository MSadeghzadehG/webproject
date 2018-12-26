

package org.elasticsearch.index.mapper;

import org.elasticsearch.plugins.MapperPlugin;
import org.elasticsearch.plugins.Plugin;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;

public class MapperExtrasPlugin extends Plugin implements MapperPlugin {

    @Override
    public Map<String, Mapper.TypeParser> getMappers() {
        Map<String, Mapper.TypeParser> mappers = new LinkedHashMap<>();
        mappers.put(ScaledFloatFieldMapper.CONTENT_TYPE, new ScaledFloatFieldMapper.TypeParser());
        mappers.put(TokenCountFieldMapper.CONTENT_TYPE, new TokenCountFieldMapper.TypeParser());
        return Collections.unmodifiableMap(mappers);
    }

}
