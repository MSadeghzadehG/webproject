

package org.elasticsearch.plugins;

import joptsimple.OptionSet;
import org.elasticsearch.Version;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.env.Environment;

import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;


class ListPluginsCommand extends EnvironmentAwareCommand {

    ListPluginsCommand() {
        super("Lists installed elasticsearch plugins");
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options, Environment env) throws Exception {
        if (Files.exists(env.pluginsFile()) == false) {
            throw new IOException("Plugins directory missing: " + env.pluginsFile());
        }

        terminal.println(Terminal.Verbosity.VERBOSE, "Plugins directory: " + env.pluginsFile());
        final List<Path> plugins = new ArrayList<>();
        try (DirectoryStream<Path> paths = Files.newDirectoryStream(env.pluginsFile())) {
            for (Path plugin : paths) {
                plugins.add(plugin);
            }
        }
        Collections.sort(plugins);
        for (final Path plugin : plugins) {
            if (MetaPluginInfo.isMetaPlugin(plugin)) {
                MetaPluginInfo metaInfo = MetaPluginInfo.readFromProperties(plugin);
                List<Path> subPluginPaths = new ArrayList<>();
                try (DirectoryStream<Path> subPaths = Files.newDirectoryStream(plugin)) {
                    for (Path subPlugin : subPaths) {
                        if (MetaPluginInfo.isPropertiesFile(subPlugin)) {
                            continue;
                        }
                        subPluginPaths.add(subPlugin);
                    }
                }
                Collections.sort(subPluginPaths);
                terminal.println(Terminal.Verbosity.SILENT, metaInfo.getName());
                for (Path subPlugin : subPluginPaths) {
                    printPlugin(env, terminal, subPlugin, "\t");
                }
            } else {
                printPlugin(env, terminal, plugin, "");
            }
        }
    }

    private void printPlugin(Environment env, Terminal terminal, Path plugin, String prefix) throws IOException {
        terminal.println(Terminal.Verbosity.SILENT, prefix + plugin.getFileName().toString());
        PluginInfo info = PluginInfo.readFromProperties(env.pluginsFile().resolve(plugin.toAbsolutePath()));
        terminal.println(Terminal.Verbosity.VERBOSE, info.toString(prefix));
        if (info.getElasticsearchVersion().equals(Version.CURRENT) == false) {
            terminal.println("WARNING: plugin [" + info.getName() + "] was built for Elasticsearch version " + info.getVersion() +
                " but version " + Version.CURRENT + " is required");
        }
    }
}
