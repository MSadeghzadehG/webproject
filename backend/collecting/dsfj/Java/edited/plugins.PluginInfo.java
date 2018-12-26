

package org.elasticsearch.plugins;

import org.elasticsearch.Version;
import org.elasticsearch.bootstrap.JarHell;
import org.elasticsearch.common.Booleans;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.common.xcontent.ToXContentObject;
import org.elasticsearch.common.xcontent.XContentBuilder;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.function.Function;
import java.util.stream.Collectors;


public class PluginInfo implements Writeable, ToXContentObject {

    public static final String ES_PLUGIN_PROPERTIES = "plugin-descriptor.properties";
    public static final String ES_PLUGIN_POLICY = "plugin-security.policy";

    private final String name;
    private final String description;
    private final String version;
    private final Version elasticsearchVersion;
    private final String javaVersion;
    private final String classname;
    private final List<String> extendedPlugins;
    private final boolean hasNativeController;

    
    public PluginInfo(String name, String description, String version, Version elasticsearchVersion, String javaVersion,
                      String classname, List<String> extendedPlugins, boolean hasNativeController) {
        this.name = name;
        this.description = description;
        this.version = version;
        this.elasticsearchVersion = elasticsearchVersion;
        this.javaVersion = javaVersion;
        this.classname = classname;
        this.extendedPlugins = Collections.unmodifiableList(extendedPlugins);
        this.hasNativeController = hasNativeController;
    }

    
    public PluginInfo(final StreamInput in) throws IOException {
        this.name = in.readString();
        this.description = in.readString();
        this.version = in.readString();
        if (in.getVersion().onOrAfter(Version.V_6_3_0)) {
            elasticsearchVersion = Version.readVersion(in);
            javaVersion = in.readString();
        } else {
                        elasticsearchVersion = in.getVersion();
                        javaVersion = "1.8";
        }
        this.classname = in.readString();
        if (in.getVersion().onOrAfter(Version.V_6_2_0)) {
            extendedPlugins = in.readList(StreamInput::readString);
        } else {
            extendedPlugins = Collections.emptyList();
        }
        if (in.getVersion().onOrAfter(Version.V_5_4_0)) {
            hasNativeController = in.readBoolean();
        } else {
            hasNativeController = false;
        }
        if (in.getVersion().onOrAfter(Version.V_6_0_0_beta2) && in.getVersion().before(Version.V_6_3_0)) {
            
            in.readBoolean();
        }
    }

    @Override
    public void writeTo(final StreamOutput out) throws IOException {
        out.writeString(name);
        out.writeString(description);
        out.writeString(version);
        if (out.getVersion().onOrAfter(Version.V_6_3_0)) {
            Version.writeVersion(elasticsearchVersion, out);
            out.writeString(javaVersion);
        }
        out.writeString(classname);
        if (out.getVersion().onOrAfter(Version.V_6_2_0)) {
            out.writeStringList(extendedPlugins);
        }
        if (out.getVersion().onOrAfter(Version.V_5_4_0)) {
            out.writeBoolean(hasNativeController);
        }
        if (out.getVersion().onOrAfter(Version.V_6_0_0_beta2) && out.getVersion().before(Version.V_6_3_0)) {
            
            out.writeBoolean(false);
        }
    }

    
    public static PluginInfo readFromProperties(final Path path) throws IOException {
        final Path descriptor = path.resolve(ES_PLUGIN_PROPERTIES);

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
                    "property [name] is missing in [" + descriptor + "]");
        }
        final String description = propsMap.remove("description");
        if (description == null) {
            throw new IllegalArgumentException(
                    "property [description] is missing for plugin [" + name + "]");
        }
        final String version = propsMap.remove("version");
        if (version == null) {
            throw new IllegalArgumentException(
                    "property [version] is missing for plugin [" + name + "]");
        }

        final String esVersionString = propsMap.remove("elasticsearch.version");
        if (esVersionString == null) {
            throw new IllegalArgumentException(
                    "property [elasticsearch.version] is missing for plugin [" + name + "]");
        }
        final Version esVersion = Version.fromString(esVersionString);
        final String javaVersionString = propsMap.remove("java.version");
        if (javaVersionString == null) {
            throw new IllegalArgumentException(
                    "property [java.version] is missing for plugin [" + name + "]");
        }
        JarHell.checkVersionFormat(javaVersionString);
        final String classname = propsMap.remove("classname");
        if (classname == null) {
            throw new IllegalArgumentException(
                    "property [classname] is missing for plugin [" + name + "]");
        }

        final String extendedString = propsMap.remove("extended.plugins");
        final List<String> extendedPlugins;
        if (extendedString == null) {
            extendedPlugins = Collections.emptyList();
        } else {
            extendedPlugins = Arrays.asList(Strings.delimitedListToStringArray(extendedString, ","));
        }

        final String hasNativeControllerValue = propsMap.remove("has.native.controller");
        final boolean hasNativeController;
        if (hasNativeControllerValue == null) {
            hasNativeController = false;
        } else {
            switch (hasNativeControllerValue) {
                case "true":
                    hasNativeController = true;
                    break;
                case "false":
                    hasNativeController = false;
                    break;
                default:
                    final String message = String.format(
                            Locale.ROOT,
                            "property [%s] must be [%s], [%s], or unspecified but was [%s]",
                            "has_native_controller",
                            "true",
                            "false",
                            hasNativeControllerValue);
                    throw new IllegalArgumentException(message);
            }
        }

        if (esVersion.before(Version.V_6_3_0) && esVersion.onOrAfter(Version.V_6_0_0_beta2)) {
            propsMap.remove("requires.keystore");
        }

        if (propsMap.isEmpty() == false) {
            throw new IllegalArgumentException("Unknown properties in plugin descriptor: " + propsMap.keySet());
        }

        return new PluginInfo(name, description, version, esVersion, javaVersionString,
                              classname, extendedPlugins, hasNativeController);
    }

    
    public String getName() {
        return name;
    }

    
    public String getDescription() {
        return description;
    }

    
    public String getClassname() {
        return classname;
    }

    
    public List<String> getExtendedPlugins() {
        return extendedPlugins;
    }

    
    public String getVersion() {
        return version;
    }

    
    public Version getElasticsearchVersion() {
        return elasticsearchVersion;
    }

    
    public String getJavaVersion() {
        return javaVersion;
    }

    
    public boolean hasNativeController() {
        return hasNativeController;
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startObject();
        {
            builder.field("name", name);
            builder.field("version", version);
            builder.field("elasticsearch_version", elasticsearchVersion);
            builder.field("java_version", javaVersion);
            builder.field("description", description);
            builder.field("classname", classname);
            builder.field("extended_plugins", extendedPlugins);
            builder.field("has_native_controller", hasNativeController);
        }
        builder.endObject();

        return builder;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        PluginInfo that = (PluginInfo) o;

        if (!name.equals(that.name)) return false;
                if (version != null ? !version.equals(that.version) : that.version != null) return false;

        return true;
    }

    @Override
    public int hashCode() {
        return name.hashCode();
    }

    @Override
    public String toString() {
        return toString("");
    }

    public String toString(String prefix) {
        final StringBuilder information = new StringBuilder()
            .append(prefix).append("- Plugin information:\n")
            .append(prefix).append("Name: ").append(name).append("\n")
            .append(prefix).append("Description: ").append(description).append("\n")
            .append(prefix).append("Version: ").append(version).append("\n")
            .append(prefix).append("Elasticsearch Version: ").append(elasticsearchVersion).append("\n")
            .append(prefix).append("Java Version: ").append(javaVersion).append("\n")
            .append(prefix).append("Native Controller: ").append(hasNativeController).append("\n")
            .append(prefix).append("Extended Plugins: ").append(extendedPlugins).append("\n")
            .append(prefix).append(" * Classname: ").append(classname);
        return information.toString();
    }
}
