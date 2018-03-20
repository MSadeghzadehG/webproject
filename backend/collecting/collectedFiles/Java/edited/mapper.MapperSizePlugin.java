

package org.elasticsearch.plugin.mapper;

import java.util.Collections;
import java.util.Map;

import org.elasticsearch.index.mapper.MetadataFieldMapper;
import org.elasticsearch.index.mapper.size.SizeFieldMapper;
import org.elasticsearch.plugins.MapperPlugin;
import org.elasticsearch.plugins.Plugin;

public class MapperSizePlugin extends Plugin implements MapperPlugin {

    @Override
    public Map<String, MetadataFieldMapper.TypeParser> getMetadataMappers() {
        return Collections.singletonMap(SizeFieldMapper.NAME, new SizeFieldMapper.TypeParser());
    }
}
