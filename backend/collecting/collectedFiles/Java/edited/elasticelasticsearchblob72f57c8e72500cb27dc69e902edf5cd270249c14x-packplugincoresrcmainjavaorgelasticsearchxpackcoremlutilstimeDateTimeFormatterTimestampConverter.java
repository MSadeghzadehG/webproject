
package org.elasticsearch.xpack.core.ml.utils.time;

import org.elasticsearch.cli.SuppressForbidden;

import java.time.DateTimeException;
import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeFormatterBuilder;
import java.time.temporal.ChronoField;
import java.time.temporal.TemporalAccessor;


public class DateTimeFormatterTimestampConverter implements TimestampConverter {
    private final DateTimeFormatter formatter;
    private final boolean hasTimeZone;
    private final ZoneId defaultZoneId;

    private DateTimeFormatterTimestampConverter(DateTimeFormatter dateTimeFormatter, boolean hasTimeZone, ZoneId defaultTimezone) {
        formatter = dateTimeFormatter;
        this.hasTimeZone = hasTimeZone;
        defaultZoneId = defaultTimezone;
    }

    
    public static TimestampConverter ofPattern(String pattern, ZoneId defaultTimezone) {
        DateTimeFormatter formatter = new DateTimeFormatterBuilder()
                .parseLenient()
                .appendPattern(pattern)
                .parseDefaulting(ChronoField.YEAR_OF_ERA, LocalDate.now(defaultTimezone).getYear())
                .toFormatter();

        String now = formatter.format(ZonedDateTime.ofInstant(Instant.ofEpochSecond(0), ZoneOffset.UTC));
        try {
            TemporalAccessor parsed = formatter.parse(now);
            boolean hasTimeZone = parsed.isSupported(ChronoField.INSTANT_SECONDS);
            if (hasTimeZone) {
                Instant.from(parsed);
            }
            else {
                LocalDateTime.from(parsed);
            }
            return new DateTimeFormatterTimestampConverter(formatter, hasTimeZone, defaultTimezone);
        }
        catch (DateTimeException e) {
            throw new IllegalArgumentException("Timestamp cannot be derived from pattern: " + pattern);
        }
    }

    @Override
    public long toEpochSeconds(String timestamp) {
        return toInstant(timestamp).getEpochSecond();
    }

    @Override
    public long toEpochMillis(String timestamp) {
        return toInstant(timestamp).toEpochMilli();
    }

    private Instant toInstant(String timestamp) {
        TemporalAccessor parsed = formatter.parse(timestamp);
        if (hasTimeZone) {
            return Instant.from(parsed);
        }
        return toInstantUnsafelyIgnoringAmbiguity(parsed);
    }

    @SuppressForbidden(reason = "TODO https:    private Instant toInstantUnsafelyIgnoringAmbiguity(TemporalAccessor parsed) {
        return LocalDateTime.from(parsed).atZone(defaultZoneId).toInstant();
    }
}
