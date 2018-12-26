

package org.elasticsearch.common.settings;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Map;

import org.elasticsearch.cli.Command;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.env.Environment;

public class CreateKeyStoreCommandTests extends KeyStoreCommandTestCase {

    @Override
    protected Command newCommand() {
        return new CreateKeyStoreCommand() {
            @Override
            protected Environment createEnv(Map<String, String> settings) throws UserException {
                return env;
            }
        };
    }

    public void testPosix() throws Exception {
        execute();
        Path configDir = env.configFile();
        assertNotNull(KeyStoreWrapper.load(configDir));
    }

    public void testNotPosix() throws Exception {
        env = setupEnv(false, fileSystems);
        execute();
        Path configDir = env.configFile();
        assertNotNull(KeyStoreWrapper.load(configDir));
    }

    public void testOverwrite() throws Exception {
        Path keystoreFile = KeyStoreWrapper.keystorePath(env.configFile());
        byte[] content = "not a keystore".getBytes(StandardCharsets.UTF_8);
        Files.write(keystoreFile, content);

        terminal.addTextInput("");         execute();
        assertArrayEquals(content, Files.readAllBytes(keystoreFile));

        terminal.addTextInput("n");         execute();
        assertArrayEquals(content, Files.readAllBytes(keystoreFile));

        terminal.addTextInput("y");
        execute();
        assertNotNull(KeyStoreWrapper.load(env.configFile()));
    }
}
