

package org.elasticsearch.client;

import java.util.HashMap;
import java.util.Map;

import joptsimple.internal.Strings;
import org.apache.http.Header;
import org.elasticsearch.test.ESTestCase;


public abstract class RestClientBuilderTestCase extends ESTestCase {
    
    public void assertHeaders(RestClient client, Map<String, String> expectedHeaders) {
        expectedHeaders = new HashMap<>(expectedHeaders);         for (Header header : client.defaultHeaders) {
            String name = header.getName();
            String expectedValue = expectedHeaders.remove(name);
            if (expectedValue == null) {
                fail("Found unexpected header in rest client: " + name);
            }
            assertEquals(expectedValue, header.getValue());
        }
        if (expectedHeaders.isEmpty() == false) {
            fail("Missing expected headers in rest client: " + Strings.join(expectedHeaders.keySet(), ", "));
        }
    }
}
