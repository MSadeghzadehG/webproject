
package com.iluwatar.module;

import java.io.PrintStream;

import org.apache.log4j.Logger;


public final class ConsoleLoggerModule {

  private static final Logger LOGGER = Logger.getLogger(ConsoleLoggerModule.class);

  private static ConsoleLoggerModule singleton = null;

  public PrintStream output = null;
  public PrintStream error = null;

  private ConsoleLoggerModule() {}

  
  public static ConsoleLoggerModule getSingleton() {

    if (ConsoleLoggerModule.singleton == null) {
      ConsoleLoggerModule.singleton = new ConsoleLoggerModule();
    }

    return ConsoleLoggerModule.singleton;
  }

  
  public ConsoleLoggerModule prepare() {

    LOGGER.debug("ConsoleLoggerModule::prepare();");

    this.output = new PrintStream(System.out);
    this.error = new PrintStream(System.err);

    return this;
  }

  
  public void unprepare() {

    if (this.output != null) {

      this.output.flush();
      this.output.close();
    }

    if (this.error != null) {

      this.error.flush();
      this.error.close();
    }

    LOGGER.debug("ConsoleLoggerModule::unprepare();");
  }

  
  public void printString(final String value) {
    this.output.println(value);
  }

  
  public void printErrorString(final String value) {
    this.error.println(value);
  }
}
