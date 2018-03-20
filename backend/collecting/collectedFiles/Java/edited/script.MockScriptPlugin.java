

package org.elasticsearch.script;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.ScriptPlugin;

import java.util.Collection;
import java.util.Map;
import java.util.function.Function;


public abstract class MockScriptPlugin extends Plugin implements ScriptPlugin {

    public static final String NAME = MockScriptEngine.NAME;

    @Override
    public ScriptEngine getScriptEngine(Settings settings, Collection<ScriptContext<?>> contexts) {
        return new MockScriptEngine(pluginScriptLang(), pluginScripts());
    }

    protected abstract Map<String, Function<Map<String, Object>, Object>> pluginScripts();

    public String pluginScriptLang() {
        return NAME;
    }
}
