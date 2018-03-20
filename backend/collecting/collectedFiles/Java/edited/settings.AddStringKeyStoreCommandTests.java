

package org.elasticsearch.common.settings;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.Map;

import org.elasticsearch.cli.Command;
import org.elasticsearch.cli.ExitCodes;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.cli.UserException;
import org.elasticsearch.env.Environment;

import static org.hamcrest.Matchers.containsString;

public class AddStringKeyStoreCommandTests extends KeyStoreCommandTestCase {
    InputStream input;

    @Override
    protected Command newCommand() {
        return new AddStringKeyStoreCommand() {
            @Override
            protected Environment createEnv(Map<String, String> settings) throws UserException {
                return env;
            }
            @Override
            InputStream getStdin() {
                return input;
            }
        };
    }

    public void testMissingPromptCreate() throws Exception {
        terminal.addTextInput("y");
        terminal.addSecretInput("bar");
        execute("foo");
        assertSecureString("foo", "bar");
    }

    public void testMissingForceCreate() throws Exception {
        terminal.addSecretInput("bar");
        execute("-f", "foo");
        assertSecureString("foo", "bar");
    }

    public void testMissingNoCreate() throws Exception {
        terminal.addTextInput("n");         execute("foo");
        assertNull(KeyStoreWrapper.load(env.configFile()));
    }

    public void testOverwritePromptDefault() throws Exception {
        createKeystore("", "foo", "bar");
        terminal.addTextInput("");
        execute("foo");
        assertSecureString("foo", "bar");
    }

    public void testOverwritePromptExplicitNo() throws Exception {
        createKeystore("", "foo", "bar");
        terminal.addTextInput("n");         execute("foo");
        assertSecureString("foo", "bar");
    }

    public void testOverwritePromptExplicitYes() throws Exception {
        createKeystore("", "foo", "bar");
        terminal.addTextInput("y");
        terminal.addSecretInput("newvalue");
        execute("foo");
        assertSecureString("foo", "newvalue");
    }

    public void testOverwriteForceShort() throws Exception {
        createKeystore("", "foo", "bar");
        terminal.addSecretInput("newvalue");
        execute("-f", "foo");         assertSecureString("foo", "newvalue");
    }

    public void testOverwriteForceLong() throws Exception {
        createKeystore("", "foo", "bar");
        terminal.addSecretInput("and yet another secret value");
        execute("--force", "foo");         assertSecureString("foo", "and yet another secret value");
    }

    public void testForceNonExistent() throws Exception {
        createKeystore("");
        terminal.addSecretInput("value");
        execute("--force", "foo");         assertSecureString("foo", "value");
    }

    public void testPromptForValue() throws Exception {
        KeyStoreWrapper.create().save(env.configFile(), new char[0]);
        terminal.addSecretInput("secret value");
        execute("foo");
        assertSecureString("foo", "secret value");
    }

    public void testStdinShort() throws Exception {
        KeyStoreWrapper.create().save(env.configFile(), new char[0]);
        setInput("secret value 1");
        execute("-x", "foo");
        assertSecureString("foo", "secret value 1");
    }

    public void testStdinLong() throws Exception {
        KeyStoreWrapper.create().save(env.configFile(), new char[0]);
        setInput("secret value 2");
        execute("--stdin", "foo");
        assertSecureString("foo", "secret value 2");
    }

    public void testMissingSettingName() throws Exception {
        createKeystore("");
        terminal.addTextInput("");
        UserException e = expectThrows(UserException.class, this::execute);
        assertEquals(ExitCodes.USAGE, e.exitCode);
        assertThat(e.getMessage(), containsString("The setting name can not be null"));
    }

    void setInput(String inputStr) {
        input = new ByteArrayInputStream(inputStr.getBytes(StandardCharsets.UTF_8));
    }
}
