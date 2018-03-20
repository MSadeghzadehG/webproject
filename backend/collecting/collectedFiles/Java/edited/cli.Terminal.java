

package org.elasticsearch.cli;

import java.io.BufferedReader;
import java.io.Console;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.nio.charset.Charset;
import java.util.Locale;


public abstract class Terminal {

    
    public static final Terminal DEFAULT = ConsoleTerminal.isSupported() ? new ConsoleTerminal() : new SystemTerminal();

    
    public enum Verbosity {
        SILENT, 
        NORMAL, 
        VERBOSE 
    }

    
    private Verbosity verbosity = Verbosity.NORMAL;

    
    private final String lineSeparator;

    protected Terminal(String lineSeparator) {
        this.lineSeparator = lineSeparator;
    }

    
    public void setVerbosity(Verbosity verbosity) {
        this.verbosity = verbosity;
    }

    
    public abstract String readText(String prompt);

    
    public abstract char[] readSecret(String prompt);

    
    public abstract PrintWriter getWriter();

    
    public final void println(String msg) {
        println(Verbosity.NORMAL, msg);
    }

    
    public final void println(Verbosity verbosity, String msg) {
        print(verbosity, msg + lineSeparator);
    }

    
    public final void print(Verbosity verbosity, String msg) {
        if (this.verbosity.ordinal() >= verbosity.ordinal()) {
            getWriter().print(msg);
            getWriter().flush();
        }
    }

    
    public final boolean promptYesNo(String prompt, boolean defaultYes) {
        String answerPrompt = defaultYes ? " [Y/n]" : " [y/N]";
        while (true) {
            String answer = readText(prompt + answerPrompt);
            if (answer == null || answer.isEmpty()) {
                return defaultYes;
            }
            answer = answer.toLowerCase(Locale.ROOT);
            boolean answerYes = answer.equals("y");
            if (answerYes == false && answer.equals("n") == false) {
                println("Did not understand answer '" + answer + "'");
                continue;
            }
            return answerYes;
        }
    }

    private static class ConsoleTerminal extends Terminal {

        private static final Console CONSOLE = System.console();

        ConsoleTerminal() {
            super(System.lineSeparator());
        }

        static boolean isSupported() {
            return CONSOLE != null;
        }

        @Override
        public PrintWriter getWriter() {
            return CONSOLE.writer();
        }

        @Override
        public String readText(String prompt) {
            return CONSOLE.readLine("%s", prompt);
        }

        @Override
        public char[] readSecret(String prompt) {
            return CONSOLE.readPassword("%s", prompt);
        }
    }

    private static class SystemTerminal extends Terminal {

        private static final PrintWriter WRITER = newWriter();

        SystemTerminal() {
            super(System.lineSeparator());
        }

        @SuppressForbidden(reason = "Writer for System.out")
        private static PrintWriter newWriter() {
            return new PrintWriter(System.out);
        }

        @Override
        public PrintWriter getWriter() {
            return WRITER;
        }

        @Override
        public String readText(String text) {
            getWriter().print(text);
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in, Charset.defaultCharset()));
            try {
                return reader.readLine();
            } catch (IOException ioe) {
                throw new RuntimeException(ioe);
            }
        }

        @Override
        public char[] readSecret(String text) {
            return readText(text).toCharArray();
        }
    }
}
