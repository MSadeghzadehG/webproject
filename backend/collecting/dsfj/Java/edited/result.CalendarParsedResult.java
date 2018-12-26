

package com.google.zxing.client.result;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public final class CalendarParsedResult extends ParsedResult {

  private static final Pattern RFC2445_DURATION =
      Pattern.compile("P(?:(\\d+)W)?(?:(\\d+)D)?(?:T(?:(\\d+)H)?(?:(\\d+)M)?(?:(\\d+)S)?)?");
  private static final long[] RFC2445_DURATION_FIELD_UNITS = {
      7 * 24 * 60 * 60 * 1000L,       24 * 60 * 60 * 1000L,       60 * 60 * 1000L,       60 * 1000L,       1000L,   };

  private static final Pattern DATE_TIME = Pattern.compile("[0-9]{8}(T[0-9]{6}Z?)?");

  private final String summary;
  private final long start;
  private final boolean startAllDay;
  private final long end;
  private final boolean endAllDay;
  private final String location;
  private final String organizer;
  private final String[] attendees;
  private final String description;
  private final double latitude;
  private final double longitude;

  public CalendarParsedResult(String summary,
                              String startString,
                              String endString,
                              String durationString,
                              String location,
                              String organizer,
                              String[] attendees,
                              String description,
                              double latitude,
                              double longitude) {
    super(ParsedResultType.CALENDAR);
    this.summary = summary;

    try {
      this.start = parseDate(startString);
    } catch (ParseException pe) {
      throw new IllegalArgumentException(pe.toString());
    }

    if (endString == null) {
      long durationMS = parseDurationMS(durationString);
      end = durationMS < 0L ? -1L : start + durationMS;
    } else {
      try {
        this.end = parseDate(endString);
      } catch (ParseException pe) {
        throw new IllegalArgumentException(pe.toString());
      }
    }

    this.startAllDay = startString.length() == 8;
    this.endAllDay = endString != null && endString.length() == 8;

    this.location = location;
    this.organizer = organizer;
    this.attendees = attendees;
    this.description = description;
    this.latitude = latitude;
    this.longitude = longitude;
  }

  public String getSummary() {
    return summary;
  }

  
  @Deprecated
  public Date getStart() {
    return new Date(start);
  }

  
  public long getStartTimestamp() {
    return start;
  }

  
  public boolean isStartAllDay() {
    return startAllDay;
  }

  
  @Deprecated
  public Date getEnd() {
    return end < 0L ? null : new Date(end);
  }

  
  public long getEndTimestamp() {
    return end;
  }

  
  public boolean isEndAllDay() {
    return endAllDay;
  }

  public String getLocation() {
    return location;
  }

  public String getOrganizer() {
    return organizer;
  }

  public String[] getAttendees() {
    return attendees;
  }

  public String getDescription() {
    return description;
  }

  public double getLatitude() {
    return latitude;
  }

  public double getLongitude() {
    return longitude;
  }

  @Override
  public String getDisplayResult() {
    StringBuilder result = new StringBuilder(100);
    maybeAppend(summary, result);
    maybeAppend(format(startAllDay, start), result);
    maybeAppend(format(endAllDay, end), result);
    maybeAppend(location, result);
    maybeAppend(organizer, result);
    maybeAppend(attendees, result);
    maybeAppend(description, result);
    return result.toString();
  }

  
  private static long parseDate(String when) throws ParseException {
    if (!DATE_TIME.matcher(when).matches()) {
      throw new ParseException(when, 0);
    }
    if (when.length() == 8) {
            DateFormat format = new SimpleDateFormat("yyyyMMdd", Locale.ENGLISH);
                        format.setTimeZone(TimeZone.getTimeZone("GMT"));
      return format.parse(when).getTime();
    }
        if (when.length() == 16 && when.charAt(15) == 'Z') {
      long milliseconds = parseDateTimeString(when.substring(0, 15));
      Calendar calendar = new GregorianCalendar();
            milliseconds += calendar.get(Calendar.ZONE_OFFSET);
                  calendar.setTime(new Date(milliseconds));
      return milliseconds + calendar.get(Calendar.DST_OFFSET);
    }
    return parseDateTimeString(when);
  }

  private static String format(boolean allDay, long date) {
    if (date < 0L) {
      return null;
    }
    DateFormat format = allDay
        ? DateFormat.getDateInstance(DateFormat.MEDIUM)
        : DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM);
    return format.format(date);
  }

  private static long parseDurationMS(CharSequence durationString) {
    if (durationString == null) {
      return -1L;
    }
    Matcher m = RFC2445_DURATION.matcher(durationString);
    if (!m.matches()) {
      return -1L;
    }
    long durationMS = 0L;
    for (int i = 0; i < RFC2445_DURATION_FIELD_UNITS.length; i++) {
      String fieldValue = m.group(i + 1);
      if (fieldValue != null) {
        durationMS += RFC2445_DURATION_FIELD_UNITS[i] * Integer.parseInt(fieldValue);
      }
    }
    return durationMS;
  }

  private static long parseDateTimeString(String dateTimeString) throws ParseException {
    DateFormat format = new SimpleDateFormat("yyyyMMdd'T'HHmmss", Locale.ENGLISH);
    return format.parse(dateTimeString).getTime();
  }

}
