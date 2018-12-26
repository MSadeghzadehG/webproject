

package org.elasticsearch.cli;

import joptsimple.OptionSet;
import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

import static org.hamcrest.CoreMatchers.containsString;
import static org.hamcrest.Matchers.isEmptyString;

public class EvilCommandTests extends ESTestCase {

    public void testCommandShutdownHook() throws Exception {
        final AtomicBoolean closed = new AtomicBoolean();
        final boolean shouldThrow = randomBoolean();
        final Command command = new Command("test-command-shutdown-hook", () -> {}) {
            @Override
            protected void execute(Terminal terminal, OptionSet options) throws Exception {

            }

            @Override
            public void close() throws IOException {
                closed.set(true);
                if (shouldThrow) {
                    throw new IOException("fail");
                }
            }
        };
        final MockTerminal terminal = new MockTerminal();
        command.main(new String[0], terminal);
        assertNotNull(command.getShutdownHookThread());
                assertTrue(Runtime.getRuntime().removeShutdownHook(command.getShutdownHookThread()));
        command.getShutdownHookThread().run();
        command.getShutdownHookThread().join();
        assertTrue(closed.get());
        final String output = terminal.getOutput();
        if (shouldThrow) {
                        assertThat(output, containsString("java.io.IOException: fail"));
                        assertThat(output, containsString("\tat org.elasticsearch.cli.EvilCommandTests$1.close"));
        } else {
            assertThat(output, isEmptyString());
        }
    }

}
