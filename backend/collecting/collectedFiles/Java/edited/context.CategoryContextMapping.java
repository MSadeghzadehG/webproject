

package org.elasticsearch.search.suggest.completion.context;

import org.apache.lucene.document.SortedDocValuesField;
import org.apache.lucene.document.SortedSetDocValuesField;
import org.apache.lucene.document.StoredField;
import org.apache.lucene.index.IndexableField;
import org.apache.lucene.search.SortedSetSortField;
import org.elasticsearch.ElasticsearchParseException;
import org.elasticsearch.Version;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.common.xcontent.XContentParser.Token;
import org.elasticsearch.index.mapper.KeywordFieldMapper;
import org.elasticsearch.index.mapper.ParseContext;
import org.elasticsearch.index.mapper.ParseContext.Document;
import org.elasticsearch.index.mapper.StringFieldType;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;


public class CategoryContextMapping extends ContextMapping<CategoryQueryContext> {

    private static final String FIELD_FIELDNAME = "path";

    static final String CONTEXT_VALUE = "context";
    static final String CONTEXT_BOOST = "boost";
    static final String CONTEXT_PREFIX = "prefix";

    private final String fieldName;

    
    private CategoryContextMapping(String name, String fieldName) {
        super(Type.CATEGORY, name);
        this.fieldName = fieldName;
    }

    
    public String getFieldName() {
        return fieldName;
    }

    
    protected static CategoryContextMapping load(String name, Map<String, Object> config) throws ElasticsearchParseException {
        CategoryContextMapping.Builder mapping = new CategoryContextMapping.Builder(name);
        Object fieldName = config.get(FIELD_FIELDNAME);
        if (fieldName != null) {
            mapping.field(fieldName.toString());
            config.remove(FIELD_FIELDNAME);
        }
        return mapping.build();
    }

    @Override
    protected XContentBuilder toInnerXContent(XContentBuilder builder, Params params) throws IOException {
        if (fieldName != null) {
            builder.field(FIELD_FIELDNAME, fieldName);
        }
        return builder;
    }

    
    @Override
    public Set<CharSequence> parseContext(ParseContext parseContext, XContentParser parser)
            throws IOException, ElasticsearchParseException {
        final Set<CharSequence> contexts = new HashSet<>();
        Token token = parser.currentToken();
        if (token == Token.VALUE_STRING || token == Token.VALUE_NUMBER || token == Token.VALUE_BOOLEAN) {
            contexts.add(parser.text());
        } else if (token == Token.START_ARRAY) {
            while ((token = parser.nextToken()) != Token.END_ARRAY) {
                if (token == Token.VALUE_STRING || token == Token.VALUE_NUMBER || token == Token.VALUE_BOOLEAN) {
                    contexts.add(parser.text());
                } else {
                    throw new ElasticsearchParseException(
                            "context array must have string, number or boolean values, but was [" + token + "]");
                }
            }
        } else {
            throw new ElasticsearchParseException(
                    "contexts must be a string, number or boolean or a list of string, number or boolean, but was [" + token + "]");
        }
        return contexts;
    }

    @Override
    public Set<CharSequence> parseContext(Document document) {
        Set<CharSequence> values = null;
        if (fieldName != null) {
            IndexableField[] fields = document.getFields(fieldName);
            values = new HashSet<>(fields.length);
            for (IndexableField field : fields) {
                if (field instanceof SortedDocValuesField ||
                        field instanceof SortedSetDocValuesField ||
                        field instanceof StoredField) {
                                    } else if (field.fieldType() instanceof KeywordFieldMapper.KeywordFieldType) {
                    values.add(field.binaryValue().utf8ToString());
                } else if (field.fieldType() instanceof StringFieldType) {
                    values.add(field.stringValue());
                } else {
                    throw new IllegalArgumentException("Failed to parse context field [" + fieldName + "], only keyword and text fields are accepted");
                }
            }
        }
        return (values == null) ? Collections.emptySet() : values;
    }

    @Override
    protected CategoryQueryContext fromXContent(XContentParser parser) throws IOException {
        return CategoryQueryContext.fromXContent(parser);
    }

    
    @Override
    public List<InternalQueryContext> toInternalQueryContexts(List<CategoryQueryContext> queryContexts) {
        List<InternalQueryContext> internalInternalQueryContexts = new ArrayList<>(queryContexts.size());
        internalInternalQueryContexts.addAll(
            queryContexts.stream()
                .map(queryContext -> new InternalQueryContext(queryContext.getCategory(), queryContext.getBoost(), queryContext.isPrefix()))
                .collect(Collectors.toList()));
        return internalInternalQueryContexts;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        if (!super.equals(o)) return false;
        CategoryContextMapping mapping = (CategoryContextMapping) o;
        return !(fieldName != null ? !fieldName.equals(mapping.fieldName) : mapping.fieldName != null);
    }

    @Override
    public int hashCode() {
        return Objects.hash(super.hashCode(), fieldName);
    }

    
    public static class Builder extends ContextBuilder<CategoryContextMapping> {

        private String fieldName;

        
        public Builder(String name) {
            super(name);
        }

        
        public Builder field(String fieldName) {
            this.fieldName = fieldName;
            return this;
        }

        @Override
        public CategoryContextMapping build() {
            return new CategoryContextMapping(name, fieldName);
        }
    }
}
