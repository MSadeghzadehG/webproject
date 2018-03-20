

package org.elasticsearch.ingest.common;

import org.elasticsearch.ingest.AbstractProcessor;
import org.elasticsearch.ingest.ConfigurationUtils;
import org.elasticsearch.ingest.IngestDocument;
import org.elasticsearch.ingest.Processor;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;


public final class SplitProcessor extends AbstractProcessor {

    public static final String TYPE = "split";

    private final String field;
    private final String separator;
    private final boolean ignoreMissing;
    private final String targetField;

    SplitProcessor(String tag, String field, String separator, boolean ignoreMissing, String targetField) {
        super(tag);
        this.field = field;
        this.separator = separator;
        this.ignoreMissing = ignoreMissing;
        this.targetField = targetField;
    }

    String getField() {
        return field;
    }

    String getSeparator() {
        return separator;
    }

    boolean isIgnoreMissing() {
        return ignoreMissing;
    }

    String getTargetField() {
        return targetField;
    }

    @Override
    public void execute(IngestDocument document) {
        String oldVal = document.getFieldValue(field, String.class, ignoreMissing);

        if (oldVal == null && ignoreMissing) {
            return;
        } else if (oldVal == null) {
            throw new IllegalArgumentException("field [" + field + "] is null, cannot split.");
        }

        String[] strings = oldVal.split(separator);
        List<String> splitList = new ArrayList<>(strings.length);
        Collections.addAll(splitList, strings);
        document.setFieldValue(targetField, splitList);
    }

    @Override
    public String getType() {
        return TYPE;
    }

    public static class Factory implements Processor.Factory {
        @Override
        public SplitProcessor create(Map<String, Processor.Factory> registry, String processorTag,
                                     Map<String, Object> config) throws Exception {
            String field = ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "field");
            boolean ignoreMissing = ConfigurationUtils.readBooleanProperty(TYPE, processorTag, config, "ignore_missing", false);
            String targetField = ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "target_field", field);
            return new SplitProcessor(processorTag, field,
                ConfigurationUtils.readStringProperty(TYPE, processorTag, config, "separator"), ignoreMissing, targetField);
        }
    }
}
