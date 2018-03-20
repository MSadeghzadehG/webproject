

package org.elasticsearch.plugin.mapper;

import java.util.Collections;
import java.util.Map;

import org.elasticsearch.index.mapper.Mapper;
import org.elasticsearch.index.mapper.murmur3.Murmur3FieldMapper;
import org.elasticsearch.plugins.MapperPlugin;
import org.elasticsearch.plugins.Plugin;

public class MapperMurmur3Plugin extends Plugin implements MapperPlugin {

    @Override
    public Map<String, Mapper.TypeParser> getMappers() {
        return Collections.singletonMap(Murmur3FieldMapper.CONTENT_TYPE, new Murmur3FieldMapper.TypeParser());
    }
}
