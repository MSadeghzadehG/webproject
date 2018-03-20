

package org.elasticsearch.plugins;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.cli.Command;
import org.elasticsearch.cli.LoggingAwareMultiCommand;
import org.elasticsearch.cli.MultiCommand;
import org.elasticsearch.cli.Terminal;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;


public class PluginCli extends LoggingAwareMultiCommand {

    private final Collection<Command> commands;

    private PluginCli() {
        super("A tool for managing installed elasticsearch plugins");
        subcommands.put("list", new ListPluginsCommand());
        subcommands.put("install", new InstallPluginCommand());
        subcommands.put("remove", new RemovePluginCommand());
        commands = Collections.unmodifiableCollection(subcommands.values());
    }

    public static void main(String[] args) throws Exception {
        exit(new PluginCli().main(args, Terminal.DEFAULT));
    }

    @Override
    public void close() throws IOException {
        IOUtils.close(commands);
    }

}
