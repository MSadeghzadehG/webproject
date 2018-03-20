

package org.elasticsearch.plugins;

import org.apache.lucene.util.LuceneTestCase;
import org.elasticsearch.Version;
import org.elasticsearch.test.ESTestCase;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.endsWith;

@LuceneTestCase.SuppressFileSystems(value = "ExtrasFS")
public class MetaPluginInfoTests extends ESTestCase {

    public void testReadFromProperties() throws Exception {
        Path pluginDir = createTempDir().resolve("fake-meta-plugin");
        PluginTestUtil.writeMetaPluginProperties(pluginDir,
            "description", "fake desc",
            "name", "my_meta_plugin");
        MetaPluginInfo info = MetaPluginInfo.readFromProperties(pluginDir);
        assertEquals("my_meta_plugin", info.getName());
        assertEquals("fake desc", info.getDescription());
    }

    public void testReadFromPropertiesNameMissing() throws Exception {
        Path pluginDir = createTempDir().resolve("fake-meta-plugin");
        PluginTestUtil.writeMetaPluginProperties(pluginDir);
        IllegalArgumentException e = expectThrows(IllegalArgumentException.class, () -> MetaPluginInfo.readFromProperties(pluginDir));
        assertThat(e.getMessage(), containsString("property [name] is missing for"));

        PluginTestUtil.writeMetaPluginProperties(pluginDir, "name", "");
        e = expectThrows(IllegalArgumentException.class, () -> MetaPluginInfo.readFromProperties(pluginDir));
        assertThat(e.getMessage(), containsString("property [name] is missing for"));
    }

    public void testReadFromPropertiesDescriptionMissing() throws Exception {
        Path pluginDir = createTempDir().resolve("fake-meta-plugin");
        PluginTestUtil.writeMetaPluginProperties(pluginDir, "name", "fake-meta-plugin");
        IllegalArgumentException e = expectThrows(IllegalArgumentException.class, () -> MetaPluginInfo.readFromProperties(pluginDir));
        assertThat(e.getMessage(), containsString("[description] is missing"));
    }

    public void testUnknownProperties() throws Exception {
        Path pluginDir = createTempDir().resolve("fake-meta-plugin");
        PluginTestUtil.writeMetaPluginProperties(pluginDir,
            "extra", "property",
            "unknown", "property",
            "description", "fake desc",
            "name", "my_meta_plugin");
        IllegalArgumentException e = expectThrows(IllegalArgumentException.class, () -> MetaPluginInfo.readFromProperties(pluginDir));
        assertThat(e.getMessage(), containsString("Unknown properties in meta plugin descriptor"));
    }

    public void testExtractAllPluginsWithDuplicates() throws Exception {
        Path pluginDir = createTempDir().resolve("plugins");
                Path plugin1 = pluginDir.resolve("plugin1");
        Files.createDirectories(plugin1);
        PluginTestUtil.writePluginProperties(plugin1,
            "description", "fake desc",
            "name", "plugin1",
            "version", "1.0",
            "elasticsearch.version", Version.CURRENT.toString(),
            "java.version", System.getProperty("java.specification.version"),
            "classname", "FakePlugin");

                Path metaPlugin = pluginDir.resolve("meta_plugin");
        Files.createDirectory(metaPlugin);
        PluginTestUtil.writeMetaPluginProperties(metaPlugin,
            "description", "fake desc",
            "name", "meta_plugin");
        Path plugin2 = metaPlugin.resolve("plugin1");
        Files.createDirectory(plugin2);
        PluginTestUtil.writePluginProperties(plugin2,
            "description", "fake desc",
            "name", "plugin1",
            "version", "1.0",
            "elasticsearch.version", Version.CURRENT.toString(),
            "java.version", System.getProperty("java.specification.version"),
            "classname", "FakePlugin");
        Path plugin3 = metaPlugin.resolve("plugin2");
        Files.createDirectory(plugin3);
        PluginTestUtil.writePluginProperties(plugin3,
            "description", "fake desc",
            "name", "plugin2",
            "version", "1.0",
            "elasticsearch.version", Version.CURRENT.toString(),
            "java.version", System.getProperty("java.specification.version"),
            "classname", "FakePlugin");

        IllegalStateException exc =
            expectThrows(IllegalStateException.class, () -> PluginsService.findPluginDirs(pluginDir));
        assertThat(exc.getMessage(), containsString("duplicate plugin"));
        assertThat(exc.getMessage(), endsWith("plugin1"));
    }
}
