

package com.google.zxing.client.android.result;

import android.content.ActivityNotFoundException;
import android.util.Log;
import com.google.zxing.client.android.R;
import com.google.zxing.client.result.CalendarParsedResult;
import com.google.zxing.client.result.ParsedResult;

import android.app.Activity;
import android.content.Intent;

import java.text.DateFormat;


public final class CalendarResultHandler extends ResultHandler {

  private static final String TAG = CalendarResultHandler.class.getSimpleName();

  private static final int[] buttons = {
      R.string.button_add_calendar
  };

  public CalendarResultHandler(Activity activity, ParsedResult result) {
    super(activity, result);
  }

  @Override
  public int getButtonCount() {
    return buttons.length;
  }

  @Override
  public int getButtonText(int index) {
    return buttons[index];
  }

  @Override
  public void handleButtonPress(int index) {
    if (index == 0) {
      CalendarParsedResult calendarResult = (CalendarParsedResult) getResult();

      String description = calendarResult.getDescription();
      String organizer = calendarResult.getOrganizer();
      if (organizer != null) {         if (description == null) {
          description = organizer;
        } else {
          description = description + '\n' + organizer;
        }
      }

      addCalendarEvent(calendarResult.getSummary(),
                       calendarResult.getStartTimestamp(),
                       calendarResult.isStartAllDay(),
                       calendarResult.getEndTimestamp(),
                       calendarResult.getLocation(),
                       description,
                       calendarResult.getAttendees());
    }
  }

  
  private void addCalendarEvent(String summary,
                                long start,
                                boolean allDay,
                                long end,
                                String location,
                                String description,
                                String[] attendees) {
    Intent intent = new Intent(Intent.ACTION_INSERT);
    intent.setType("vnd.android.cursor.item/event");
    intent.putExtra("beginTime", start);
    if (allDay) {
      intent.putExtra("allDay", true);
    }
    if (end < 0L) {
      if (allDay) {
                end = start + 24 * 60 * 60 * 1000;
      } else {
        end = start;
      }
    }
    intent.putExtra("endTime", end);
    intent.putExtra("title", summary);
    intent.putExtra("eventLocation", location);
    intent.putExtra("description", description);
    if (attendees != null) {
      intent.putExtra(Intent.EXTRA_EMAIL, attendees);
          }

    try {
            rawLaunchIntent(intent);
    } catch (ActivityNotFoundException anfe) {
      Log.w(TAG, "No calendar app available that responds to " + Intent.ACTION_INSERT);
            intent.setAction(Intent.ACTION_EDIT);
      launchIntent(intent);     }
  }


  @Override
  public CharSequence getDisplayContents() {

    CalendarParsedResult calResult = (CalendarParsedResult) getResult();
    StringBuilder result = new StringBuilder(100);

    ParsedResult.maybeAppend(calResult.getSummary(), result);

    long start = calResult.getStartTimestamp();
    ParsedResult.maybeAppend(format(calResult.isStartAllDay(), start), result);

    long end = calResult.getEndTimestamp();
    if (end >= 0L) {
      if (calResult.isEndAllDay() && start != end) {
                                        end -= 24 * 60 * 60 * 1000;
      }
      ParsedResult.maybeAppend(format(calResult.isEndAllDay(), end), result);
    }

    ParsedResult.maybeAppend(calResult.getLocation(), result);
    ParsedResult.maybeAppend(calResult.getOrganizer(), result);
    ParsedResult.maybeAppend(calResult.getAttendees(), result);
    ParsedResult.maybeAppend(calResult.getDescription(), result);
    return result.toString();
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

  @Override
  public int getDisplayTitle() {
    return R.string.result_calendar;
  }
}
