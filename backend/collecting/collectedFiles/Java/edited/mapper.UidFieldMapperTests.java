

package org.elasticsearch.index.mapper;

import org.apache.lucene.index.IndexOptions;
import org.apache.lucene.index.IndexableField;
import org.elasticsearch.Version;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.common.compress.CompressedXContent;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.index.mapper.MapperService.MergeReason;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESSingleNodeTestCase;
import org.elasticsearch.test.InternalSettingsPlugin;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

public class UidFieldMapperTests extends ESSingleNodeTestCase {

    @Override
    protected Collection<Class<? extends Plugin>> getPlugins() {
        return Collections.singleton(InternalSettingsPlugin.class);
    }

    public void testDefaultsMultipleTypes() throws IOException {
        Settings indexSettings = Settings.builder()
                .put("index.version.created", Version.V_5_6_0)
                .build();
        MapperService mapperService = createIndex("test", indexSettings).mapperService();
        DocumentMapper mapper = mapperService.merge("type", new CompressedXContent("{\"type\":{}}"), MergeReason.MAPPING_UPDATE);
        ParsedDocument document = mapper.parse(SourceToParse.source("index", "type", "id", new BytesArray("{}"), XContentType.JSON));
        IndexableField[] fields = document.rootDoc().getFields(UidFieldMapper.NAME);
        assertEquals(1, fields.length);
        assertEquals(IndexOptions.DOCS, fields[0].fieldType().indexOptions());
        assertTrue(fields[0].fieldType().stored());
        assertEquals("type#id", fields[0].stringValue());
    }

    public void testDefaultsSingleType() throws IOException {
        Settings indexSettings = Settings.EMPTY;
        MapperService mapperService = createIndex("test", indexSettings).mapperService();
        DocumentMapper mapper = mapperService.merge("type", new CompressedXContent("{\"type\":{}}"), MergeReason.MAPPING_UPDATE);
        ParsedDocument document = mapper.parse(SourceToParse.source("index", "type", "id", new BytesArray("{}"), XContentType.JSON));
        assertEquals(Collections.<IndexableField>emptyList(), Arrays.asList(document.rootDoc().getFields(UidFieldMapper.NAME)));
    }
}
