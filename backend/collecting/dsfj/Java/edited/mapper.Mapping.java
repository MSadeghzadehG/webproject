

package org.elasticsearch.index.mapper;

import org.elasticsearch.Version;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.xcontent.ToXContent;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentFactory;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import static java.util.Collections.emptyMap;
import static java.util.Collections.unmodifiableMap;


public final class Mapping implements ToXContentFragment {

    final Version indexCreated;
    final RootObjectMapper root;
    final MetadataFieldMapper[] metadataMappers;
    final Map<Class<? extends MetadataFieldMapper>, MetadataFieldMapper> metadataMappersMap;
    final Map<String, Object> meta;

    public Mapping(Version indexCreated, RootObjectMapper rootObjectMapper, MetadataFieldMapper[] metadataMappers, Map<String, Object> meta) {
        this.indexCreated = indexCreated;
        this.metadataMappers = metadataMappers;
        Map<Class<? extends MetadataFieldMapper>, MetadataFieldMapper> metadataMappersMap = new HashMap<>();
        for (MetadataFieldMapper metadataMapper : metadataMappers) {
            metadataMappersMap.put(metadataMapper.getClass(), metadataMapper);
        }
        this.root = rootObjectMapper;
                Arrays.sort(metadataMappers, new Comparator<Mapper>() {
            @Override
            public int compare(Mapper o1, Mapper o2) {
                return o1.name().compareTo(o2.name());
            }
        });
        this.metadataMappersMap = unmodifiableMap(metadataMappersMap);
        this.meta = meta;
    }

    
    public RootObjectMapper root() {
        return root;
    }

    
    public Mapping mappingUpdate(Mapper rootObjectMapper) {
        return new Mapping(indexCreated, (RootObjectMapper) rootObjectMapper, metadataMappers, meta);
    }

    
    @SuppressWarnings("unchecked")
    public <T extends MetadataFieldMapper> T metadataMapper(Class<T> clazz) {
        return (T) metadataMappersMap.get(clazz);
    }

    
    public Mapping merge(Mapping mergeWith) {
        RootObjectMapper mergedRoot = root.merge(mergeWith.root);
        Map<Class<? extends MetadataFieldMapper>, MetadataFieldMapper> mergedMetaDataMappers = new HashMap<>(metadataMappersMap);
        for (MetadataFieldMapper metaMergeWith : mergeWith.metadataMappers) {
            MetadataFieldMapper mergeInto = mergedMetaDataMappers.get(metaMergeWith.getClass());
            MetadataFieldMapper merged;
            if (mergeInto == null) {
                merged = metaMergeWith;
            } else {
                merged = mergeInto.merge(metaMergeWith);
            }
            mergedMetaDataMappers.put(merged.getClass(), merged);
        }
        Map<String, Object> mergedMeta = mergeWith.meta == null ? meta : mergeWith.meta;
        return new Mapping(indexCreated, mergedRoot, mergedMetaDataMappers.values().toArray(new MetadataFieldMapper[0]), mergedMeta);
    }

    
    public Mapping updateFieldType(Map<String, MappedFieldType> fullNameToFieldType) {
        MetadataFieldMapper[] updatedMeta = null;
        for (int i = 0; i < metadataMappers.length; ++i) {
            MetadataFieldMapper currentFieldMapper = metadataMappers[i];
            MetadataFieldMapper updatedFieldMapper = (MetadataFieldMapper) currentFieldMapper.updateFieldType(fullNameToFieldType);
            if (updatedFieldMapper != currentFieldMapper) {
                if (updatedMeta == null) {
                    updatedMeta = Arrays.copyOf(metadataMappers, metadataMappers.length);
                }
                updatedMeta[i] = updatedFieldMapper;
            }
        }
        RootObjectMapper updatedRoot = root.updateFieldType(fullNameToFieldType);
        if (updatedMeta == null && updatedRoot == root) {
            return this;
        }
        return new Mapping(indexCreated, updatedRoot, updatedMeta == null ? metadataMappers : updatedMeta, meta);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        root.toXContent(builder, params, new ToXContent() {
            @Override
            public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
                if (meta != null) {
                    builder.field("_meta", meta);
                }
                for (Mapper mapper : metadataMappers) {
                    mapper.toXContent(builder, params);
                }
                return builder;
            }
        });
        return builder;
    }

    @Override
    public String toString() {
        try {
            XContentBuilder builder = XContentFactory.jsonBuilder().startObject();
            toXContent(builder, new ToXContent.MapParams(emptyMap()));
            return Strings.toString(builder.endObject());
        } catch (IOException bogus) {
            throw new UncheckedIOException(bogus);
        }
    }
}
