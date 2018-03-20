

package org.elasticsearch.common.settings;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.env.Environment;


class AddStringKeyStoreCommand extends EnvironmentAwareCommand {

    private final OptionSpec<Void> stdinOption;
    private final OptionSpec<Void> forceOption;
    private final OptionSpec<String> arguments;

    AddStringKeyStoreCommand() {
        super("Add a string setting to the keystore");
        this.stdinOption = parser.acceptsAll(Arrays.asList("x", "stdin"), "Read setting value from stdin");
        this.forceOption = parser.acceptsAll(Arrays.asList("f", "force"), "Overwrite existing setting without prompting");
        this.arguments = parser.nonOptions("setting name");
    }

        InputStream getStdin() {
        return System.in;
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options, Environment env) throws Exception {
        KeyStoreWrapper keystore = KeyStoreWrapper.load(env.configFile());
        if (keystore == null) {
            if (options.has(forceOption) == false &&
                terminal.promptYesNo("The elasticsearch keystore does not exist. Do you want to create it?", false) == false) {
                terminal.println("Exiting without creating keystore.");
                return;
            }
            keystore = KeyStoreWrapper.create();
            keystore.save(env.configFile(), new char[0] );
            terminal.println("Created elasticsearch keystore in " + env.configFile());
        } else {
            keystore.decrypt(new char[0] );
        }

        String setting = arguments.value(options);
        if (setting == null) {
            throw new UserException(ExitCodes.USAGE, "The setting name can not be null");
        }
        if (keystore.getSettingNames().contains(setting) && options.has(forceOption) == false) {
            if (terminal.promptYesNo("Setting " + setting + " already exists. Overwrite?", false) == false) {
                terminal.println("Exiting without modifying keystore.");
                return;
            }
        }

        final char[] value;
        if (options.has(stdinOption)) {
            BufferedReader stdinReader = new BufferedReader(new InputStreamReader(getStdin(), StandardCharsets.UTF_8));
            value = stdinReader.readLine().toCharArray();
        } else {
            value = terminal.readSecret("Enter value for " + setting + ": ");
        }

        try {
            keystore.setString(setting, value);
        } catch (IllegalArgumentException e) {
            throw new UserException(ExitCodes.DATA_ERROR, "String value must contain only ASCII");
        }
        keystore.save(env.configFile(), new char[0]);
    }
}
