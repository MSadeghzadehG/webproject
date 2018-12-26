

package org.elasticsearch.index.reindex;

import org.elasticsearch.script.ExecutableScript;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

import static org.elasticsearch.test.ESTestCase.randomBoolean;

public class SimpleExecutableScript implements ExecutableScript {
    private final Consumer<Map<String, Object>> script;
    private Map<String, Object> ctx;

    public SimpleExecutableScript(Consumer<Map<String, Object>> script) {
        this.script = script;
    }

    @Override
    public Object run() {
        script.accept(ctx);
        return null;
    }

    @Override
    @SuppressWarnings("unchecked")
    public void setNextVar(String name, Object value) {
        if ("ctx".equals(name)) {
            ctx = (Map<String, Object>) value;
        } else {
            throw new IllegalArgumentException("Unsupported var [" + name + "]");
        }
    }
}
