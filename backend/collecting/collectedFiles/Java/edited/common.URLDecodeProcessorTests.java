

package org.elasticsearch.ingest.common;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

public class URLDecodeProcessorTests extends AbstractStringProcessorTestCase {
    @Override
    protected String modifyInput(String input) {
        return "Hello%20G%C3%BCnter" + input;
    }

    @Override
    protected AbstractStringProcessor newProcessor(String field, boolean ignoreMissing, String targetField) {
        return new URLDecodeProcessor(randomAlphaOfLength(10), field, ignoreMissing, targetField);
    }

    @Override
    protected String expectedResult(String input) {
        try {
            return "Hello Günter" + URLDecoder.decode(input, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException("invalid");
        }
    }
}
