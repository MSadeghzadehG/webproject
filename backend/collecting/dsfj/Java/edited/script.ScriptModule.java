

package org.elasticsearch.script;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.elasticsearch.common.settings.ClusterSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.ScriptPlugin;


public class ScriptModule {

    public static final Map<String, ScriptContext<?>> CORE_CONTEXTS;
    static {
        CORE_CONTEXTS = Stream.of(
            SearchScript.CONTEXT,
            SearchScript.AGGS_CONTEXT,
            ExecutableScript.CONTEXT,
            ExecutableScript.AGGS_CONTEXT,
            ExecutableScript.UPDATE_CONTEXT,
            ExecutableScript.INGEST_CONTEXT,
            FilterScript.CONTEXT,
            SimilarityScript.CONTEXT,
            SimilarityWeightScript.CONTEXT,
            TemplateScript.CONTEXT
        ).collect(Collectors.toMap(c -> c.name, Function.identity()));
    }

    private final ScriptService scriptService;

    public ScriptModule(Settings settings, List<ScriptPlugin> scriptPlugins) {
        Map<String, ScriptEngine> engines = new HashMap<>();
        Map<String, ScriptContext<?>> contexts = new HashMap<>(CORE_CONTEXTS);
        for (ScriptPlugin plugin : scriptPlugins) {
            for (ScriptContext context : plugin.getContexts()) {
                ScriptContext oldContext = contexts.put(context.name, context);
                if (oldContext != null) {
                    throw new IllegalArgumentException("Context name [" + context.name + "] defined twice");
                }
            }
        }
        for (ScriptPlugin plugin : scriptPlugins) {
            ScriptEngine engine = plugin.getScriptEngine(settings, contexts.values());
            if (engine != null) {
                ScriptEngine existing = engines.put(engine.getType(), engine);
                if (existing != null) {
                    throw new IllegalArgumentException("scripting language [" + engine.getType() + "] defined for engine [" +
                        existing.getClass().getName() + "] and [" + engine.getClass().getName());
                }
            }
        }
        scriptService = new ScriptService(settings, Collections.unmodifiableMap(engines), Collections.unmodifiableMap(contexts));
    }

    
    public ScriptService getScriptService() {
        return scriptService;
    }

    
    public void registerClusterSettingsListeners(ClusterSettings clusterSettings) {
        scriptService.registerClusterSettingsListeners(clusterSettings);
    }
}
