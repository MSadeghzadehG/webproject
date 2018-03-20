

package org.elasticsearch.common.inject.internal;

import java.util.LinkedHashMap;
import java.util.Map;


public class ToStringBuilder {

        final Map<String, Object> map = new LinkedHashMap<>();

    final String name;

    public ToStringBuilder(String name) {
        this.name = name;
    }

    public ToStringBuilder(Class type) {
        this.name = type.getSimpleName();
    }

    public ToStringBuilder add(String name, Object value) {
        if (map.put(name, value) != null) {
            throw new RuntimeException("Duplicate names: " + name);
        }
        return this;
    }

    @Override
    public String toString() {
        return name + map.toString().replace('{', '[').replace('}', ']');
    }
}
