

package com.google.zxing.web;

import javax.servlet.ServletContext;
import java.util.logging.Formatter;
import java.util.logging.Handler;
import java.util.logging.LogRecord;


final class ServletContextLogHandler extends Handler {

  private final ServletContext context;

  ServletContextLogHandler(ServletContext context) {
    this.context = context;
  }

  @Override
  public void publish(LogRecord record) {
    Formatter formatter = getFormatter();
    String message;
    if (formatter == null) {
      message = record.getMessage();
    } else {
      message = formatter.format(record);
    }
    Throwable throwable = record.getThrown();
    if (throwable == null) {
      context.log(message);
    } else {
      context.log(message, throwable);
    }
  }

  @Override
  public void flush() {
      }

  @Override
  public void close() {
      }

}