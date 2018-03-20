

package org.elasticsearch.common.logging;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.elasticsearch.Build;
import org.elasticsearch.Version;
import org.elasticsearch.common.SuppressLoggerChecks;
import org.elasticsearch.common.util.concurrent.ThreadContext;

import java.io.CharArrayWriter;
import java.nio.charset.Charset;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeFormatterBuilder;
import java.time.format.SignStyle;
import java.util.BitSet;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static java.time.temporal.ChronoField.DAY_OF_MONTH;
import static java.time.temporal.ChronoField.DAY_OF_WEEK;
import static java.time.temporal.ChronoField.HOUR_OF_DAY;
import static java.time.temporal.ChronoField.MINUTE_OF_HOUR;
import static java.time.temporal.ChronoField.MONTH_OF_YEAR;
import static java.time.temporal.ChronoField.SECOND_OF_MINUTE;
import static java.time.temporal.ChronoField.YEAR;


public class DeprecationLogger {

    private final Logger logger;

    
    private static final CopyOnWriteArraySet<ThreadContext> THREAD_CONTEXT = new CopyOnWriteArraySet<>();

    
    public static void setThreadContext(ThreadContext threadContext) {
        Objects.requireNonNull(threadContext, "Cannot register a null ThreadContext");

                if (THREAD_CONTEXT.add(threadContext) == false) {
            throw new IllegalStateException("Double-setting ThreadContext not allowed!");
        }
    }

    
    public static void removeThreadContext(ThreadContext threadContext) {
        assert threadContext != null;

                if (THREAD_CONTEXT.remove(threadContext) == false) {
            throw new IllegalStateException("Removing unknown ThreadContext not allowed!");
        }
    }

    
    public DeprecationLogger(Logger parentLogger) {
        String name = parentLogger.getName();
        if (name.startsWith("org.elasticsearch")) {
            name = name.replace("org.elasticsearch.", "org.elasticsearch.deprecation.");
        } else {
            name = "deprecation." + name;
        }
        this.logger = LogManager.getLogger(name);
    }

    
    public void deprecated(String msg, Object... params) {
        deprecated(THREAD_CONTEXT, msg, params);
    }

        private Set<String> keys = Collections.newSetFromMap(Collections.synchronizedMap(new LinkedHashMap<String, Boolean>() {
        @Override
        protected boolean removeEldestEntry(final Map.Entry eldest) {
            return size() > 128;
        }
    }));

    
    public void deprecatedAndMaybeLog(final String key, final String msg, final Object... params) {
        deprecated(THREAD_CONTEXT, msg, keys.add(key), params);
    }

    
    private static final String WARNING_FORMAT =
            String.format(
                    Locale.ROOT,
                    "299 Elasticsearch-%s%s-%s ",
                    Version.CURRENT.toString(),
                    Build.CURRENT.isSnapshot() ? "-SNAPSHOT" : "",
                    Build.CURRENT.shortHash()) +
                    "\"%s\" \"%s\"";

    
    private static final DateTimeFormatter RFC_7231_DATE_TIME;

    static {
        final Map<Long, String> dow = new HashMap<>();
        dow.put(1L, "Mon");
        dow.put(2L, "Tue");
        dow.put(3L, "Wed");
        dow.put(4L, "Thu");
        dow.put(5L, "Fri");
        dow.put(6L, "Sat");
        dow.put(7L, "Sun");
        final Map<Long, String> moy = new HashMap<>();
        moy.put(1L, "Jan");
        moy.put(2L, "Feb");
        moy.put(3L, "Mar");
        moy.put(4L, "Apr");
        moy.put(5L, "May");
        moy.put(6L, "Jun");
        moy.put(7L, "Jul");
        moy.put(8L, "Aug");
        moy.put(9L, "Sep");
        moy.put(10L, "Oct");
        moy.put(11L, "Nov");
        moy.put(12L, "Dec");
        RFC_7231_DATE_TIME = new DateTimeFormatterBuilder()
                .parseCaseInsensitive()
                .parseLenient()
                .optionalStart()
                .appendText(DAY_OF_WEEK, dow)
                .appendLiteral(", ")
                .optionalEnd()
                .appendValue(DAY_OF_MONTH, 2, 2, SignStyle.NOT_NEGATIVE)
                .appendLiteral(' ')
                .appendText(MONTH_OF_YEAR, moy)
                .appendLiteral(' ')
                .appendValue(YEAR, 4)
                .appendLiteral(' ')
                .appendValue(HOUR_OF_DAY, 2)
                .appendLiteral(':')
                .appendValue(MINUTE_OF_HOUR, 2)
                .optionalStart()
                .appendLiteral(':')
                .appendValue(SECOND_OF_MINUTE, 2)
                .optionalEnd()
                .appendLiteral(' ')
                .appendOffset("+HHMM", "GMT")
                .toFormatter(Locale.getDefault(Locale.Category.FORMAT));
    }

    private static final ZoneId GMT = ZoneId.of("GMT");

    
    public static Pattern WARNING_HEADER_PATTERN = Pattern.compile(
            "299 " +                     "Elasticsearch-\\d+\\.\\d+\\.\\d+(?:-(?:alpha|beta|rc)\\d+)?(?:-SNAPSHOT)?-(?:[a-f0-9]{7}|Unknown) " +                     "\"((?:\t| |!|[\\x23-\\x5B]|[\\x5D-\\x7E]|[\\x80-\\xFF]|\\\\|\\\\\")*)\" " +                                         "\"" +                     "(?:Mon|Tue|Wed|Thu|Fri|Sat|Sun), " +                     "\\d{2} " +                     "(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) " +                     "\\d{4} " +                     "\\d{2}:\\d{2}:\\d{2} " +                     "GMT" +                     "\""); 
    
    public static String extractWarningValueFromWarningHeader(final String s) {
        
        final int firstQuote = s.indexOf('\"');
        final int lastQuote = s.lastIndexOf('\"');
        final int penultimateQuote = s.lastIndexOf('\"', lastQuote - 1);
        final String warningValue = s.substring(firstQuote + 1, penultimateQuote - 2);
        assert assertWarningValue(s, warningValue);
        return warningValue;
    }

    
    private static boolean assertWarningValue(final String s, final String warningValue) {
        final Matcher matcher = WARNING_HEADER_PATTERN.matcher(s);
        final boolean matches = matcher.matches();
        assert matches;
        return matcher.group(1).equals(warningValue);
    }

    
    void deprecated(final Set<ThreadContext> threadContexts, final String message, final Object... params) {
        deprecated(threadContexts, message, true, params);
    }

    @SuppressLoggerChecks(reason = "safely delegates to logger")
    void deprecated(final Set<ThreadContext> threadContexts, final String message, final boolean log, final Object... params) {
        final Iterator<ThreadContext> iterator = threadContexts.iterator();

        if (iterator.hasNext()) {
            final String formattedMessage = LoggerMessageFormat.format(message, params);
            final String warningHeaderValue = formatWarning(formattedMessage);
            assert WARNING_HEADER_PATTERN.matcher(warningHeaderValue).matches();
            assert extractWarningValueFromWarningHeader(warningHeaderValue).equals(escapeAndEncode(formattedMessage));
            while (iterator.hasNext()) {
                try {
                    final ThreadContext next = iterator.next();
                    next.addResponseHeader("Warning", warningHeaderValue, DeprecationLogger::extractWarningValueFromWarningHeader);
                } catch (final IllegalStateException e) {
                                    }
            }
        }

        if (log) {
            logger.warn(message, params);
        }
    }

    
    public static String formatWarning(final String s) {
        return String.format(Locale.ROOT, WARNING_FORMAT, escapeAndEncode(s), RFC_7231_DATE_TIME.format(ZonedDateTime.now(GMT)));
    }

    
    public static String escapeAndEncode(final String s) {
        return encode(escapeBackslashesAndQuotes(s));
    }

    
    static String escapeBackslashesAndQuotes(final String s) {
        return s.replaceAll("([\"\\\\])", "\\\\$1");
    }

    private static BitSet doesNotNeedEncoding;

    static {
        doesNotNeedEncoding = new BitSet(1 + 0xFF);
        doesNotNeedEncoding.set('\t');
        doesNotNeedEncoding.set(' ');
        doesNotNeedEncoding.set('!');
        doesNotNeedEncoding.set('\\');
        doesNotNeedEncoding.set('"');
                for (int i = 0x23; i <= 0x24; i++) {
            doesNotNeedEncoding.set(i);
        }
        for (int i = 0x26; i <= 0x5B; i++) {
            doesNotNeedEncoding.set(i);
        }
        for (int i = 0x5D; i <= 0x7E; i++) {
            doesNotNeedEncoding.set(i);
        }
        for (int i = 0x80; i <= 0xFF; i++) {
            doesNotNeedEncoding.set(i);
        }
        assert !doesNotNeedEncoding.get('%');
    }

    private static final Charset UTF_8 = Charset.forName("UTF-8");

    
    static String encode(final String s) {
        final StringBuilder sb = new StringBuilder(s.length());
        boolean encodingNeeded = false;
        for (int i = 0; i < s.length();) {
            int current = (int) s.charAt(i);
            
            if (doesNotNeedEncoding.get(current)) {
                                sb.append((char) current);
                i++;
            } else {
                int startIndex = i;
                do {
                    i++;
                } while (i < s.length() && !doesNotNeedEncoding.get(s.charAt(i)));

                final byte[] bytes = s.substring(startIndex, i).getBytes(UTF_8);
                                for (int j = 0; j < bytes.length; j++) {
                    sb.append('%').append(hex(bytes[j] >> 4)).append(hex(bytes[j]));
                }
                encodingNeeded = true;
            }
        }
        return encodingNeeded ? sb.toString() : s;
    }

    private static char hex(int b) {
        final char ch = Character.forDigit(b & 0xF, 16);
        if (Character.isLetter(ch)) {
            return Character.toUpperCase(ch);
        } else {
            return ch;
        }
    }

}
