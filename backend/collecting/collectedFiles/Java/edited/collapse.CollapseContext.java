
package org.elasticsearch.search.collapse;

import org.apache.lucene.search.Sort;
import org.apache.lucene.search.grouping.CollapsingTopDocsCollector;
import org.elasticsearch.index.mapper.KeywordFieldMapper;
import org.elasticsearch.index.mapper.MappedFieldType;
import org.elasticsearch.index.mapper.NumberFieldMapper;
import org.elasticsearch.index.query.InnerHitBuilder;

import java.io.IOException;
import java.util.Collections;
import java.util.List;


public class CollapseContext {
    private final MappedFieldType fieldType;
    private final List<InnerHitBuilder> innerHits;

    public CollapseContext(MappedFieldType fieldType, InnerHitBuilder innerHit) {
        this.fieldType = fieldType;
        this.innerHits = Collections.singletonList(innerHit);
    }

    public CollapseContext(MappedFieldType fieldType, List<InnerHitBuilder> innerHits) {
        this.fieldType = fieldType;
        this.innerHits = innerHits;
    }

    
    public MappedFieldType getFieldType() {
        return fieldType;
    }

    
    public List<InnerHitBuilder> getInnerHit() {
        return innerHits;
    }

    public CollapsingTopDocsCollector<?> createTopDocs(Sort sort, int topN, boolean trackMaxScore) {
        if (fieldType instanceof KeywordFieldMapper.KeywordFieldType) {
            return CollapsingTopDocsCollector.createKeyword(fieldType.name(), sort, topN, trackMaxScore);
        } else if (fieldType instanceof NumberFieldMapper.NumberFieldType) {
            return CollapsingTopDocsCollector.createNumeric(fieldType.name(), sort, topN, trackMaxScore);
        } else {
            throw new IllegalStateException("unknown type for collapse field " + fieldType.name() +
                ", only keywords and numbers are accepted");
        }
    }
}
