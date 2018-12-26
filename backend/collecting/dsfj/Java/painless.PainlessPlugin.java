/*
 * Licensed to Elasticsearch under one or more contributor
 * license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright
 * ownership. Elasticsearch licenses this file to you under
 * the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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

/**
 * Registers Painless as a plugin.
 */
public final class PainlessPlugin extends Plugin implements ScriptPlugin, ExtensiblePlugin {

    private final Map<ScriptContext<?>, List<Whitelist>> extendedWhitelists = new HashMap<>();

    @Override
    public ScriptEngine getScriptEngine(Settings settings, Collection<ScriptContext<?>> contexts) {
        Map<ScriptContext<?>, List<Whitelist>> contextsWithWhitelists = new HashMap<>();
        for (ScriptContext<?> context : contexts) {
            // we might have a context that only uses the base whitelists, so would not have been filled in by reloadSPI
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
