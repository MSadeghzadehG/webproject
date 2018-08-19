
package org.elasticsearch.xpack.sql.cli.command;

import org.elasticsearch.xpack.sql.cli.Cli;
import org.elasticsearch.xpack.sql.cli.CliTerminal;
import org.elasticsearch.xpack.sql.cli.FatalCliException;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class PrintLogoCommand extends AbstractCliCommand {

    public PrintLogoCommand() {
        super(Pattern.compile("logo", Pattern.CASE_INSENSITIVE));
    }

    @Override
    protected boolean doHandle(CliTerminal terminal, CliSession cliSession, Matcher m, String line) {
        printLogo(terminal);
        return true;
    }

    public void printLogo(CliTerminal terminal) {
        terminal.clear();
        try (InputStream in = Cli.class.getResourceAsStream("/logo.txt")) {
            if (in == null) {
                throw new FatalCliException("Could not find logo!");
            }
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    terminal.println(line);
                }
            }
        } catch (IOException e) {
            throw new FatalCliException("Could not load logo!", e);
        }

        terminal.println();
    }

}