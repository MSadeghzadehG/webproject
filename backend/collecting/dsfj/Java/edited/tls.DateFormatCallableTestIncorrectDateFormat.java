

package com.iluwatar.tls;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.fail;


public class DateFormatCallableTestIncorrectDateFormat {

    
  static Result result;

  
  static List<String> createdExceptions = new ArrayList<String>();

  
  int expectedCounterDateValues = 0;

  
  int expectedCounterExceptions = 5;

  
  List<String> expectedExceptions = Arrays.asList("class java.text.ParseException: Unparseable date: \"15.12.2015\"",
      "class java.text.ParseException: Unparseable date: \"15.12.2015\"",
      "class java.text.ParseException: Unparseable date: \"15.12.2015\"",
      "class java.text.ParseException: Unparseable date: \"15.12.2015\"",
      "class java.text.ParseException: Unparseable date: \"15.12.2015\"");

  
  @BeforeAll
  public static void setup() {
        DateFormatCallable callableDf = new DateFormatCallable("dd/MM/yyyy", "15.12.2015");
        ExecutorService executor = Executors.newCachedThreadPool();
    Future<Result> futureResult = executor.submit(callableDf);
    try {
      result = futureResult.get();
    } catch (Exception e) {
      fail("Setup failed: " + e);
    }
    executor.shutdown();
  }

  
  @Test
  public void testExecptions() {
    assertEquals(expectedExceptions, result.getExceptionList());
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
