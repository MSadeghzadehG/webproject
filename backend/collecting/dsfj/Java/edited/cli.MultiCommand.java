

package org.elasticsearch.cli;

import java.io.Closeable;
import java.io.IOException;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;

import joptsimple.NonOptionArgumentSpec;
import joptsimple.OptionSet;

import org.elasticsearch.core.internal.io.IOUtils;


public class MultiCommand extends Command {

    protected final Map<String, Command> subcommands = new LinkedHashMap<>();

    private final NonOptionArgumentSpec<String> arguments = parser.nonOptions("command");

    
    public MultiCommand(final String description, final Runnable beforeMain) {
        super(description, beforeMain);
        parser.posixlyCorrect(true);
    }

    @Override
    protected void printAdditionalHelp(Terminal terminal) {
        if (subcommands.isEmpty()) {
            throw new IllegalStateException("No subcommands configured");
        }
        terminal.println("Commands");
        terminal.println("--------");
        for (Map.Entry<String, Command> subcommand : subcommands.entrySet()) {
            terminal.println(subcommand.getKey() + " - " + subcommand.getValue().description);
        }
        terminal.println("");
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options) throws Exception {
        if (subcommands.isEmpty()) {
            throw new IllegalStateException("No subcommands configured");
        }
        String[] args = arguments.values(options).toArray(new String[0]);
        if (args.length == 0) {
            throw new UserException(ExitCodes.USAGE, "Missing command");
        }
        Command subcommand = subcommands.get(args[0]);
        if (subcommand == null) {
            throw new UserException(ExitCodes.USAGE, "Unknown command [" + args[0] + "]");
        }
        subcommand.mainWithoutErrorHandling(Arrays.copyOfRange(args, 1, args.length), terminal);
    }

    @Override
    public void close() throws IOException {
        IOUtils.close(subcommands.values());
    }

}
