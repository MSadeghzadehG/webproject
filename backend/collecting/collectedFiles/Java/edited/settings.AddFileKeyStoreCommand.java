

package org.elasticsearch.common.settings;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.List;

import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.env.Environment;


class AddFileKeyStoreCommand extends EnvironmentAwareCommand {

    private final OptionSpec<Void> forceOption;
    private final OptionSpec<String> arguments;

    AddFileKeyStoreCommand() {
        super("Add a file setting to the keystore");
        this.forceOption = parser.acceptsAll(Arrays.asList("f", "force"), "Overwrite existing setting without prompting");
                                this.arguments = parser.nonOptions("setting [filepath]");
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

        List<String> argumentValues = arguments.values(options);
        if (argumentValues.size() == 0) {
            throw new UserException(ExitCodes.USAGE, "Missing setting name");
        }
        String setting = argumentValues.get(0);
        if (keystore.getSettingNames().contains(setting) && options.has(forceOption) == false) {
            if (terminal.promptYesNo("Setting " + setting + " already exists. Overwrite?", false) == false) {
                terminal.println("Exiting without modifying keystore.");
                return;
            }
        }

        if (argumentValues.size() == 1) {
            throw new UserException(ExitCodes.USAGE, "Missing file name");
        }
        Path file = getPath(argumentValues.get(1));
        if (Files.exists(file) == false) {
            throw new UserException(ExitCodes.IO_ERROR, "File [" + file.toString() + "] does not exist");
        }
        if (argumentValues.size() > 2) {
            throw new UserException(ExitCodes.USAGE, "Unrecognized extra arguments [" +
                String.join(", ", argumentValues.subList(2, argumentValues.size())) + "] after filepath");
        }
        keystore.setFile(setting, Files.readAllBytes(file));
        keystore.save(env.configFile(), new char[0]);
    }

    @SuppressForbidden(reason="file arg for cli")
    private Path getPath(String file) {
        return PathUtils.get(file);
    }
}
