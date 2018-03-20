

package com.iluwatar.tls;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.fail;


public class DateFormatCallableTestMultiThread {

    
  static Result[] result = new Result[4];

  
  @SuppressWarnings("serial")
  static class StringArrayList extends ArrayList<String> {
    
  }
  static List<String>[] createdDateValues = new StringArrayList[4];

  
  int expectedCounterDateValues = 5;

  
  int expectedCounterExceptions = 0;

  
  List<String> expectedDateValues = Arrays.asList("15.11.2015", "15.11.2015", "15.11.2015", "15.11.2015", "15.11.2015");

  
  @BeforeAll
  public static void setup() {
        DateFormatCallable callableDf = new DateFormatCallable("dd/MM/yyyy", "15/12/2015");
        ExecutorService executor = Executors.newCachedThreadPool();
    Future<Result> futureResult1 = executor.submit(callableDf);
    Future<Result> futureResult2 = executor.submit(callableDf);
    Future<Result> futureResult3 = executor.submit(callableDf);
    Future<Result> futureResult4 = executor.submit(callableDf);
    try {
      result[0] = futureResult1.get();
      result[1] = futureResult2.get();
      result[2] = futureResult3.get();
      result[3] = futureResult4.get();
      for (int i = 0; i < result.length; i++) {
        createdDateValues[i] = convertDatesToString(result[i]);
      }
    } catch (Exception e) {
      fail("Setup failed: " + e);
    }
    executor.shutdown();
  }

  private static List<String> convertDatesToString(Result res) {
        if (res == null || res.getDateList() == null || res.getDateList().size() == 0) {
      return null;
    }
    List<String> returnList = new StringArrayList();

    for (Date dt : res.getDateList()) {
      Calendar cal = Calendar.getInstance();
      cal.setTime(dt);
      returnList.add(cal.get(Calendar.DAY_OF_MONTH) + "." + cal.get(Calendar.MONTH) + "." + cal.get(Calendar.YEAR));
    }
    return returnList;
  }

  
  @Test
  public void testDateValues() {
    for (int i = 0; i < createdDateValues.length; i++) {
      assertEquals(expectedDateValues, createdDateValues[i]);
    }
  }

  
  @Test
  public void testCounterDateValues() {
    for (int i = 0; i < result.length; i++) {
      assertEquals(expectedCounterDateValues, result[i].getDateList().size());
    }
  }

  
  @Test
  public void testCounterExceptions() {
    for (int i = 0; i < result.length; i++) {
      assertEquals(expectedCounterExceptions, result[i].getExceptionList().size());
    }
  }
}
