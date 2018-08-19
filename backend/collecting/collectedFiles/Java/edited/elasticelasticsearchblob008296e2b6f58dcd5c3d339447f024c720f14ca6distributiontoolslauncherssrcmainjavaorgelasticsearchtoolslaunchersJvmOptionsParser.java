

package org.elasticsearch.tools.launchers;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


final class JvmOptionsParser {

    
    public static void main(final String[] args) throws IOException {
        if (args.length != 1) {
            throw new IllegalArgumentException("expected one argument specifying path to jvm.options but was " + Arrays.toString(args));
        }
        final List<String> jvmOptions = new ArrayList<>();
        final SortedMap<Integer, String> invalidLines = new TreeMap<>();
        try (InputStream is = Files.newInputStream(Paths.get(args[0]));
             Reader reader = new InputStreamReader(is, Charset.forName("UTF-8"));
             BufferedReader br = new BufferedReader(reader)) {
            parse(
                    JavaVersion.majorVersion(JavaVersion.CURRENT),
                    br,
                    new JvmOptionConsumer() {
                        @Override
                        public void accept(final String jvmOption) {
                            jvmOptions.add(jvmOption);
                        }
                    },
                    new InvalidLineConsumer() {
                        @Override
                        public void accept(final int lineNumber, final String line) {
                            invalidLines.put(lineNumber, line);
                        }
                    });
        }

        if (invalidLines.isEmpty()) {
            final String spaceDelimitedJvmOptions = spaceDelimitJvmOptions(jvmOptions);
            Launchers.outPrintln(spaceDelimitedJvmOptions);
            Launchers.exit(0);
        } else {
            final String errorMessage = String.format(
                    Locale.ROOT,
                    "encountered [%d] error%s parsing [%s]",
                    invalidLines.size(),
                    invalidLines.size() == 1 ? "" : "s",
                    args[0]);
            Launchers.errPrintln(errorMessage);
            int count = 0;
            for (final Map.Entry<Integer, String> entry : invalidLines.entrySet()) {
                count++;
                final String message = String.format(
                        Locale.ROOT,
                        "[%d]: encountered improperly formatted JVM option line [%s] on line number [%d]",
                        count,
                        entry.getValue(),
                        entry.getKey());
                Launchers.errPrintln(message);
            }
            Launchers.exit(1);
        }
    }

    
    interface JvmOptionConsumer {
        
        void accept(String jvmOption);
    }

    
    interface InvalidLineConsumer {
        
        void accept(int lineNumber, String line);
    }

    private static final Pattern PATTERN = Pattern.compile("((?<start>\\d+)(?<range>-)?(?<end>\\d+)?:)?(?<option>-.*)$");

    
    static void parse(
            final int javaMajorVersion,
            final BufferedReader br,
            final JvmOptionConsumer jvmOptionConsumer,
            final InvalidLineConsumer invalidLineConsumer) throws IOException {
        int lineNumber = 0;
        while (true) {
            final String line = br.readLine();
            lineNumber++;
            if (line == null) {
                break;
            }
            if (line.startsWith("#")) {
                                continue;
            }
            if (line.matches("\\s*")) {
                                continue;
            }
            final Matcher matcher = PATTERN.matcher(line);
            if (matcher.matches()) {
                final String start = matcher.group("start");
                final String end = matcher.group("end");
                if (start == null) {
                                        jvmOptionConsumer.accept(line);
                } else {
                    final int lower;
                    try {
                        lower = Integer.parseInt(start);
                    } catch (final NumberFormatException e) {
                        invalidLineConsumer.accept(lineNumber, line);
                        continue;
                    }
                    final int upper;
                    if (matcher.group("range") == null) {
                                                upper = lower;
                    } else if (end == null) {
                                                upper = Integer.MAX_VALUE;
                    } else {
                                                try {
                            upper = Integer.parseInt(end);
                        } catch (final NumberFormatException e) {
                            invalidLineConsumer.accept(lineNumber, line);
                            continue;
                        }
                        if (upper < lower) {
                            invalidLineConsumer.accept(lineNumber, line);
                            continue;
                        }
                    }
                    if (lower <= javaMajorVersion && javaMajorVersion <= upper) {
                        jvmOptionConsumer.accept(matcher.group("option"));
                    }
                }
            } else {
                invalidLineConsumer.accept(lineNumber, line);
            }
        }
    }

    
    static String spaceDelimitJvmOptions(final List<String> jvmOptions) {
        final StringBuilder spaceDelimitedJvmOptionsBuilder = new StringBuilder();
        final Iterator<String> it = jvmOptions.iterator();
        while (it.hasNext()) {
            spaceDelimitedJvmOptionsBuilder.append(it.next());
            if (it.hasNext()) {
                spaceDelimitedJvmOptionsBuilder.append(" ");
            }
        }
        return spaceDelimitedJvmOptionsBuilder.toString();
    }

}
