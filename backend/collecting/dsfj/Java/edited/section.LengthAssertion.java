
package org.elasticsearch.test.rest.yaml.section;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.xcontent.XContentLocation;
import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.util.List;
import java.util.Map;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.junit.Assert.assertThat;


public class LengthAssertion extends Assertion {
    public static LengthAssertion parse(XContentParser parser) throws IOException {
        XContentLocation location = parser.getTokenLocation();
        Tuple<String,Object> stringObjectTuple = ParserUtils.parseTuple(parser);
        assert stringObjectTuple.v2() != null;
        int value;
        if (stringObjectTuple.v2() instanceof Number) {
            value = ((Number) stringObjectTuple.v2()).intValue();
        } else {
            try {
                value = Integer.valueOf(stringObjectTuple.v2().toString());
            } catch(NumberFormatException e) {
                throw new IllegalArgumentException("length is not a valid number", e);
            }
        }
        return new LengthAssertion(location, stringObjectTuple.v1(), value);
    }

    private static final Logger logger = Loggers.getLogger(LengthAssertion.class);

    public LengthAssertion(XContentLocation location, String field, Object expectedValue) {
        super(location, field, expectedValue);
    }

    @Override
    protected void doAssert(Object actualValue, Object expectedValue) {
        logger.trace("assert that [{}] has length [{}] (field: [{}])", actualValue, expectedValue, getField());
        assertThat("expected value of [" + getField() + "] is not numeric (got [" + expectedValue.getClass() + "]",
                expectedValue, instanceOf(Number.class));
        int length = ((Number) expectedValue).intValue();
        if (actualValue instanceof String) {
            assertThat(errorMessage(), ((String) actualValue).length(), equalTo(length));
        } else if (actualValue instanceof List) {
            assertThat(errorMessage(), ((List) actualValue).size(), equalTo(length));
        } else if (actualValue instanceof Map) {
            assertThat(errorMessage(), ((Map) actualValue).keySet().size(), equalTo(length));
        } else {
            throw new UnsupportedOperationException("value is of unsupported type [" + safeClass(actualValue) + "]");
        }
    }

    private String errorMessage() {
        return "field [" + getField() + "] doesn't have length [" + getExpectedValue() + "]";
    }
}
