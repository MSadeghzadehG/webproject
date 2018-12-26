

package org.elasticsearch.ingest.common;

import java.util.Locale;
import java.util.Map;



public final class LowercaseProcessor extends AbstractStringProcessor {

    public static final String TYPE = "lowercase";

    LowercaseProcessor(String processorTag, String field, boolean ignoreMissing, String targetField) {
        super(processorTag, field, ignoreMissing, targetField);
    }

    @Override
    protected String process(String value) {
        return value.toLowerCase(Locale.ROOT);
    }

    @Override
    public String getType() {
        return TYPE;
    }

    public static final class Factory extends AbstractStringProcessor.Factory {

        public Factory() {
            super(TYPE);
        }

        @Override
        protected LowercaseProcessor newProcessor(String tag, Map<String, Object> config, String field,
                                                  boolean ignoreMissing, String targetField) {
            return new LowercaseProcessor(tag, field, ignoreMissing, targetField);
        }
    }
}
