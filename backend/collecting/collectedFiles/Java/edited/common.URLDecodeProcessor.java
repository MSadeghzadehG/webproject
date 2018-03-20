

package org.elasticsearch.ingest.common;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.Map;


public final class URLDecodeProcessor extends AbstractStringProcessor {

    public static final String TYPE = "urldecode";

    URLDecodeProcessor(String processorTag, String field, boolean ignoreMissing, String targetField) {
        super(processorTag, field, ignoreMissing, targetField);
    }

    @Override
    protected String process(String value) {
        try {
            return URLDecoder.decode(value, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException("could not URL-decode field[" + getField() + "]", e);
        }
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
        protected URLDecodeProcessor newProcessor(String tag, Map<String, Object> config, String field,
                                                  boolean ignoreMissing, String targetField) {
            return new URLDecodeProcessor(tag, field, ignoreMissing, targetField);
        }
    }
}
