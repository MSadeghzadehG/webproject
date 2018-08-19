
package org.elasticsearch.test.rest.yaml.section;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static java.util.Collections.unmodifiableMap;


public class ApiCallSection {

    private final String api;
    private final Map<String, String> params = new HashMap<>();
    private final Map<String, String> headers = new HashMap<>();
    private final List<Map<String, Object>> bodies = new ArrayList<>();

    public ApiCallSection(String api) {
        this.api = api;
    }

    public String getApi() {
        return api;
    }

    public Map<String, String> getParams() {
                return unmodifiableMap(params);
    }

    public void addParam(String key, String value) {
        String existingValue = params.get(key);
        if (existingValue != null) {
            value = existingValue + "," + value;
        }
        this.params.put(key, value);
    }

    public void addHeaders(Map<String, String> otherHeaders) {
        this.headers.putAll(otherHeaders);
    }

    public Map<String, String> getHeaders() {
        return unmodifiableMap(headers);
    }

    public List<Map<String, Object>> getBodies() {
        return Collections.unmodifiableList(bodies);
    }

    public void addBody(Map<String, Object> body) {
        this.bodies.add(body);
    }

    public boolean hasBody() {
        return bodies.size() > 0;
    }
}
