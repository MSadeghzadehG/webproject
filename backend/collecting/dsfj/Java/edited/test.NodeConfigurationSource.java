
package org.elasticsearch.test;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.Plugin;

import java.nio.file.Path;
import java.util.Collection;
import java.util.Collections;

public abstract class NodeConfigurationSource {

    public static final NodeConfigurationSource EMPTY = new NodeConfigurationSource() {
        @Override
        public Settings nodeSettings(int nodeOrdinal) {
            return Settings.EMPTY;
        }

        @Override
        public Path nodeConfigPath(int nodeOrdinal) {
            return null;
        }

        @Override
        public Settings transportClientSettings() {
            return Settings.EMPTY;
        }
    };

    
    public abstract Settings nodeSettings(int nodeOrdinal);

    public abstract Path nodeConfigPath(int nodeOrdinal);

    
    public Collection<Class<? extends Plugin>> nodePlugins() {
        return Collections.emptyList();
    }

    public Settings transportClientSettings() {
        return Settings.EMPTY;
    }

    
    public Collection<Class<? extends Plugin>> transportClientPlugins() {
        return Collections.emptyList();
    }

}
