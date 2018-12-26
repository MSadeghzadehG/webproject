

package org.elasticsearch.painless;


import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.painless.spi.PainlessExtension;
import org.elasticsearch.painless.spi.Whitelist;
import org.elasticsearch.plugins.ExtensiblePlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.ScriptPlugin;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.ScriptEngine;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.ServiceLoader;


public final class PainlessPlugin extends Plugin implements ScriptPlugin, ExtensiblePlugin {

    private final Map<ScriptContext<?>, List<Whitelist>> extendedWhitelists = new HashMap<>();

    @Override
    public ScriptEngine getScriptEngine(Settings settings, Collection<ScriptContext<?>> contexts) {
        Map<ScriptContext<?>, List<Whitelist>> contextsWithWhitelists = new HashMap<>();
        for (ScriptContext<?> context : contexts) {
                        List<Whitelist> whitelists = extendedWhitelists.get(context);
            if (whitelists == null) {
                whitelists = new ArrayList<>(Whitelist.BASE_WHITELISTS);
            }
            contextsWithWhitelists.put(context, whitelists);
        }
        return new PainlessScriptEngine(settings, contextsWithWhitelists);
    }

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(CompilerSettings.REGEX_ENABLED);
    }

    @Override
    public void reloadSPI(ClassLoader loader) {
        for (PainlessExtension extension : ServiceLoader.load(PainlessExtension.class, loader)) {
            for (Map.Entry<ScriptContext<?>, List<Whitelist>> entry : extension.getContextWhitelists().entrySet()) {
                List<Whitelist> existing = extendedWhitelists.computeIfAbsent(entry.getKey(),
                    c -> new ArrayList<>(Whitelist.BASE_WHITELISTS));
                existing.addAll(entry.getValue());
            }
        }
    }
}
