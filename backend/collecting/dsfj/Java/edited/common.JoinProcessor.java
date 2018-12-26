

package org.elasticsearch.ingest.common;

import org.elasticsearch.ingest.AbstractProcessor;
import org.elasticsearch.ingest.ConfigurationUtils;
import org.elasticsearch.ingest.IngestDocument;
import org.elasticsearch.ingest.Processor;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;


public final class JoinProcessor extends AbstractProcessor {

    public static final String TYPE = "join";

    private final String field;
    private final String separator;
    private final String targetField;

    JoinProcessor(String tag, String field, String separator, String targetField) {
        super(tag);
        this.field = field;
        this.separator = separator;
        this.targetField = targetField;
    }

    String getField() {
        return field;
    }

    String getSeparator() {
        return separator;
    }

    String getTargetField() {
        return targetField;
    }

    @Override
    public void execute(IngestDocument document) {
        List<?> list = document.getFieldValue(field, List.class);
        if (list == null) {
            throw new IllegalArgumentException("field [" + field + "] is null, cannot join.");
        }
        String joined = list.stream()
                .map(Object::toString)
                .collect(Collectors.joining(separator));
        document.setFieldValue(targetField, joined);
    }

    @Override
    public String getType() {
        return TYPE;
    }

    public static final class Factory implements Processor.Factory {
        @Override
        public JoinProcessor create(Map<String, Processor.Factory> registry, String processorTag,
                                    Map<String, Object> config) throws Exception {
            String field = ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "field");
            String separator = ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "separator");
            String targetField = ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "target_field", field);
            return new JoinProcessor(processorTag, field, separator, targetField);
        }
    }
}

