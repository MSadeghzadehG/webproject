

package com.iluwatar.tls;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.concurrent.Callable;


public class DateFormatCallable implements Callable<Result> {
    private ThreadLocal<DateFormat> df;      
  private String dateValue;   

  
  public DateFormatCallable(String inDateFormat, String inDateValue) {
    final String idf = inDateFormat;                     this.df = new ThreadLocal<DateFormat>() {              @Override                                            protected DateFormat initialValue() {                  return new SimpleDateFormat(idf);                  }                                                  };                                                       this.dateValue = inDateValue;
  }

  
  @Override
  public Result call() {
    System.out.println(Thread.currentThread() + " started executing...");
    Result result = new Result();

        for (int i = 1; i <= 5; i++) {
      try {
                                result.getDateList().add(this.df.get().parse(this.dateValue));         } catch (Exception e) {
                result.getExceptionList().add(e.getClass() + ": " + e.getMessage());
      }

    }

    System.out.println(Thread.currentThread() + " finished processing part of the thread");

    return result;
  }
}
