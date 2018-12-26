
package com.iluwatar.module;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;

import org.apache.log4j.Logger;


public final class FileLoggerModule {

  private static final Logger LOGGER = Logger.getLogger(FileLoggerModule.class);

  private static FileLoggerModule singleton = null;

  private static final String OUTPUT_FILE = "output.txt";
  private static final String ERROR_FILE = "error.txt";

  public PrintStream output = null;
  public PrintStream error = null;

  private FileLoggerModule() {}

  
  public static FileLoggerModule getSingleton() {

    if (FileLoggerModule.singleton == null) {
      FileLoggerModule.singleton = new FileLoggerModule();
    }

    return FileLoggerModule.singleton;
  }

  
  public FileLoggerModule prepare() throws FileNotFoundException {

    LOGGER.debug("FileLoggerModule::prepare();");

    this.output = new PrintStream(new FileOutputStream(OUTPUT_FILE));
    this.error = new PrintStream(new FileOutputStream(ERROR_FILE));

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

    LOGGER.debug("FileLoggerModule::unprepare();");
  }

  
  public void printString(final String value) {
    this.output.println(value);
  }

  
  public void printErrorString(final String value) {
    this.error.println(value);
  }
}
