

package com.iluwatar.tls;

import java.util.Calendar;
import java.util.Date;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;


public class App {
  
  public static void main(String[] args) {
    int counterDateValues = 0;
    int counterExceptions = 0;

        DateFormatCallable callableDf = new DateFormatCallable("dd/MM/yyyy", "15/12/2015");
        ExecutorService executor = Executors.newCachedThreadPool();

    Future<Result> futureResult1 = executor.submit(callableDf);
    Future<Result> futureResult2 = executor.submit(callableDf);
    Future<Result> futureResult3 = executor.submit(callableDf);
    Future<Result> futureResult4 = executor.submit(callableDf);
    try {
      Result[] result = new Result[4];
      result[0] = futureResult1.get();
      result[1] = futureResult2.get();
      result[2] = futureResult3.get();
      result[3] = futureResult4.get();

                  for (int i = 0; i < result.length; i++) {
        counterDateValues = counterDateValues + printAndCountDates(result[i]);
        counterExceptions = counterExceptions + printAndCountExceptions(result[i]);
      }

                  System.out.println("The List dateList contains " + counterDateValues + " date values");
      System.out.println("The List exceptionList contains " + counterExceptions + " exceptions");

    } catch (Exception e) {
      System.out.println("Abnormal end of program. Program throws exception: " + e); 
    }
    executor.shutdown();
  }

  
  private static int printAndCountDates(Result res) {
        int counter = 0;
    for (Date dt : res.getDateList()) {
      counter++;
      Calendar cal = Calendar.getInstance();
      cal.setTime(dt);
            System.out.println(
          cal.get(Calendar.DAY_OF_MONTH) + "." + cal.get(Calendar.MONTH) + "." + +cal.get(Calendar.YEAR));
    }
    return counter;
  }

  
  private static int printAndCountExceptions(Result res) {
        int counter = 0;
    for (String ex : res.getExceptionList()) {
      counter++;
      System.out.println(ex);
    }
    return counter;
  }
}
