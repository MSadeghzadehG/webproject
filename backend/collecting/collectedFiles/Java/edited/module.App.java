
package com.iluwatar.module;

import java.io.FileNotFoundException;


public final class App {

  public static FileLoggerModule fileLoggerModule;
  public static ConsoleLoggerModule consoleLoggerModule;

  
  public static void prepare() throws FileNotFoundException {

    
    fileLoggerModule = FileLoggerModule.getSingleton().prepare();
    consoleLoggerModule = ConsoleLoggerModule.getSingleton().prepare();
  }

  
  public static void unprepare() {

    
    fileLoggerModule.unprepare();
    consoleLoggerModule.unprepare();
  }

  
  public static void execute(final String... args) {

    
    fileLoggerModule.printString("Message");
    fileLoggerModule.printErrorString("Error");

    
    consoleLoggerModule.printString("Message");
    consoleLoggerModule.printErrorString("Error");
  }

  
  public static void main(final String... args) throws FileNotFoundException {
    prepare();
    execute(args);
    unprepare();
  }

  private App() {}
}
