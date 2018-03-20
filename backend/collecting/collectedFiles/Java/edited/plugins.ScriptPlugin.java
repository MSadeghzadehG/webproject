
package org.elasticsearch.plugins;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.ScriptEngine;


public interface ScriptPlugin {

    
    default ScriptEngine getScriptEngine(Settings settings, Collection<ScriptContext<?>> contexts) {
        return null;
    }

    
    default List<ScriptContext> getContexts() {
        return Collections.emptyList();
    }
}
