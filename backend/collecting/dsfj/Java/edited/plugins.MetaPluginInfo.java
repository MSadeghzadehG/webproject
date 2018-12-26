

package org.elasticsearch.plugins;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Map;
import java.util.Properties;
import java.util.function.Function;
import java.util.stream.Collectors;


public class MetaPluginInfo {
    static final String ES_META_PLUGIN_PROPERTIES = "meta-plugin-descriptor.properties";

    private final String name;
    private final String description;

    
    private MetaPluginInfo(String name, String description) {
        this.name = name;
        this.description = description;
    }

    
    public static boolean isMetaPlugin(final Path path) {
        return Files.exists(path.resolve(ES_META_PLUGIN_PROPERTIES));
    }

    
    public static boolean isPropertiesFile(final Path path) {
        return ES_META_PLUGIN_PROPERTIES.equals(path.getFileName().toString());
    }

    

    
    public static MetaPluginInfo readFromProperties(final Path path) throws IOException {
        final Path descriptor = path.resolve(ES_META_PLUGIN_PROPERTIES);

        final Map<String, String> propsMap;
        {
            final Properties props = new Properties();
            try (InputStream stream = Files.newInputStream(descriptor)) {
                props.load(stream);
            }
            propsMap = props.stringPropertyNames().stream().collect(Collectors.toMap(Function.identity(), props::getProperty));
        }

        final String name = propsMap.remove("name");
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException(
                    "property [name] is missing for meta plugin in [" + descriptor + "]");
        }
        final String description = propsMap.remove("description");
        if (description == null) {
            throw new IllegalArgumentException(
                    "property [description] is missing for meta plugin [" + name + "]");
        }

        if (propsMap.isEmpty() == false) {
            throw new IllegalArgumentException("Unknown properties in meta plugin descriptor: " + propsMap.keySet());
        }

        return new MetaPluginInfo(name, description);
    }

    
    public String getName() {
        return name;
    }

    
    public String getDescription() {
        return description;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        MetaPluginInfo that = (MetaPluginInfo) o;

        if (!name.equals(that.name)) return false;

        return true;
    }

    @Override
    public int hashCode() {
        return name.hashCode();
    }

    @Override
    public String toString() {
        final StringBuilder information = new StringBuilder()
                .append("- Plugin information:\n")
                .append("Name: ").append(name).append("\n")
                .append("Description: ").append(description);
        return information.toString();
    }

}
