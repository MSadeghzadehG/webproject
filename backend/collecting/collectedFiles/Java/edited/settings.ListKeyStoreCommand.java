

package org.elasticsearch.common.settings;


import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import joptsimple.OptionSet;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.env.Environment;


class ListKeyStoreCommand extends EnvironmentAwareCommand {

    ListKeyStoreCommand() {
        super("List entries in the keystore");
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options, Environment env) throws Exception {
        KeyStoreWrapper keystore = KeyStoreWrapper.load(env.configFile());
        if (keystore == null) {
            throw new UserException(ExitCodes.DATA_ERROR, "Elasticsearch keystore not found. Use 'create' command to create one.");
        }

        keystore.decrypt(new char[0] );

        List<String> sortedEntries = new ArrayList<>(keystore.getSettingNames());
        Collections.sort(sortedEntries);
        for (String entry : sortedEntries) {
            terminal.println(entry);
        }
    }
}
