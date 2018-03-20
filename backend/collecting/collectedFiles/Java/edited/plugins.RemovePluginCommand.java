

package org.elasticsearch.plugins;

import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.env.Environment;

import java.io.IOException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static org.elasticsearch.cli.Terminal.Verbosity.VERBOSE;


class RemovePluginCommand extends EnvironmentAwareCommand {

        
    static final int PLUGIN_STILL_USED = 11;

    private final OptionSpec<Void> purgeOption;
    private final OptionSpec<String> arguments;

    RemovePluginCommand() {
        super("removes a plugin from Elasticsearch");
        this.purgeOption = parser.acceptsAll(Arrays.asList("p", "purge"), "Purge plugin configuration files");
        this.arguments = parser.nonOptions("plugin name");
    }

    @Override
    protected void execute(final Terminal terminal, final OptionSet options, final Environment env) throws Exception {
        final String pluginName = arguments.value(options);
        final boolean purge = options.has(purgeOption);
        execute(terminal, env, pluginName, purge);
    }

    
    void execute(Terminal terminal, Environment env, String pluginName, boolean purge) throws IOException, UserException {
        if (pluginName == null) {
            throw new UserException(ExitCodes.USAGE, "plugin name is required");
        }

                List<String> usedBy = new ArrayList<>();
        Set<PluginsService.Bundle> bundles = PluginsService.getPluginBundles(env.pluginsFile());
        for (PluginsService.Bundle bundle : bundles) {
            for (String extendedPlugin : bundle.plugin.getExtendedPlugins()) {
                if (extendedPlugin.equals(pluginName)) {
                    usedBy.add(bundle.plugin.getName());
                }
            }
        }
        if (usedBy.isEmpty() == false) {
            throw new UserException(PLUGIN_STILL_USED, "plugin [" + pluginName + "] cannot be removed" +
                " because it is extended by other plugins: " + usedBy);
        }

        final Path pluginDir = env.pluginsFile().resolve(pluginName);
        final Path pluginConfigDir = env.configFile().resolve(pluginName);
        final Path removing = env.pluginsFile().resolve(".removing-" + pluginName);

        terminal.println("-> removing [" + pluginName + "]...");
        
        if ((!Files.exists(pluginDir) && !Files.exists(pluginConfigDir) && !Files.exists(removing))
                || (!Files.exists(pluginDir) && Files.exists(pluginConfigDir) && !purge)) {
            final String message = String.format(
                    Locale.ROOT, "plugin [%s] not found; run 'elasticsearch-plugin list' to get list of installed plugins", pluginName);
            throw new UserException(ExitCodes.CONFIG, message);
        }

        final List<Path> pluginPaths = new ArrayList<>();

        
        if (Files.exists(pluginDir)) {
            try (Stream<Path> paths = Files.list(pluginDir)) {
                pluginPaths.addAll(paths.collect(Collectors.toList()));
            }
            terminal.println(VERBOSE, "removing [" + pluginDir + "]");
        }

        final Path pluginBinDir = env.binFile().resolve(pluginName);
        if (Files.exists(pluginBinDir)) {
            if (!Files.isDirectory(pluginBinDir)) {
                throw new UserException(ExitCodes.IO_ERROR, "bin dir for " + pluginName + " is not a directory");
            }
            try (Stream<Path> paths = Files.list(pluginBinDir)) {
                pluginPaths.addAll(paths.collect(Collectors.toList()));
            }
            pluginPaths.add(pluginBinDir);
            terminal.println(VERBOSE, "removing [" + pluginBinDir + "]");
        }

        if (Files.exists(pluginConfigDir)) {
            if (purge) {
                try (Stream<Path> paths = Files.list(pluginConfigDir)) {
                    pluginPaths.addAll(paths.collect(Collectors.toList()));
                }
                pluginPaths.add(pluginConfigDir);
                terminal.println(VERBOSE, "removing [" + pluginConfigDir + "]");
            } else {
                
                final String message = String.format(
                        Locale.ROOT,
                        "-> preserving plugin config files [%s] in case of upgrade; use --purge if not needed",
                        pluginConfigDir);
                terminal.println(message);
            }
        }

        
        try {
            Files.createFile(removing);
        } catch (final FileAlreadyExistsException e) {
            
            terminal.println(VERBOSE, "marker file [" + removing + "] already exists");
        }

                pluginPaths.add(pluginDir);

                pluginPaths.add(removing);

        IOUtils.rm(pluginPaths.toArray(new Path[pluginPaths.size()]));
    }

}
