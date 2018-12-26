

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


public class DateFormatCallableTest {

    
  static Result result;

  
  static List<String> createdDateValues = new ArrayList<String>();

  
  int expectedCounterDateValues = 5;

  
  int expectedCounterExceptions = 0;

  
  List<String> expectedDateValues = Arrays.asList("15.11.2015", "15.11.2015", "15.11.2015", "15.11.2015", "15.11.2015");

  
  @BeforeAll
  public static void setup() {
        DateFormatCallable callableDf = new DateFormatCallable("dd/MM/yyyy", "15/12/2015");
        ExecutorService executor = Executors.newCachedThreadPool();
    Future<Result> futureResult = executor.submit(callableDf);
    try {
      result = futureResult.get();
      createdDateValues = convertDatesToString(result);
    } catch (Exception e) {
      fail("Setup failed: " + e);
    }
    executor.shutdown();
  }

  private static List<String> convertDatesToString(Result res) {
        if (res == null || res.getDateList() == null || res.getDateList().size() == 0) {
      return null;
    }
    List<String> returnList = new ArrayList<String>();

    for (Date dt : res.getDateList()) {
      Calendar cal = Calendar.getInstance();
      cal.setTime(dt);
      returnList.add(cal.get(Calendar.DAY_OF_MONTH) + "." + cal.get(Calendar.MONTH) + "." + cal.get(Calendar.YEAR));
    }
    return returnList;
  }

  
  @Test
  public void testDateValues() {
    assertEquals(expectedDateValues, createdDateValues);
  }

  
  @Test
  public void testCounterDateValues() {
    assertEquals(expectedCounterDateValues, result.getDateList().size());
  }

  
  @Test
  public void testCounterExceptions() {
    assertEquals(expectedCounterExceptions, result.getExceptionList().size());
  }
}
