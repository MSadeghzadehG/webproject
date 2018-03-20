

package org.elasticsearch.painless;

import org.elasticsearch.painless.api.Debug;
import org.elasticsearch.script.ScriptException;

import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import static java.util.Collections.singletonList;


public class PainlessExplainError extends Error {
    private final Object objectToExplain;

    public PainlessExplainError(Object objectToExplain) {
        this.objectToExplain = objectToExplain;
    }

    Object getObjectToExplain() {
        return objectToExplain;
    }

    
    public Map<String, List<String>> getHeaders(Definition definition) {
        Map<String, List<String>> headers = new TreeMap<>();
        String toString = "null";
        String javaClassName = null;
        String painlessClassName = null;
        if (objectToExplain != null) {
            toString = objectToExplain.toString();
            javaClassName = objectToExplain.getClass().getName();
            Definition.Struct struct = definition.ClassToType(objectToExplain.getClass()).struct;
            if (struct != null) {
                painlessClassName = struct.name;
            }
        }

        headers.put("es.to_string", singletonList(toString));
        if (painlessClassName != null) {
            headers.put("es.painless_class", singletonList(painlessClassName));
        }
        if (javaClassName != null) {
            headers.put("es.java_class", singletonList(javaClassName));
        }
        return headers;
    }
}
