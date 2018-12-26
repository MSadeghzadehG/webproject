
package com.iluwatar.module;

import org.apache.log4j.Logger;
import org.junit.jupiter.api.Test;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import static org.junit.jupiter.api.Assertions.assertEquals;


public final class FileLoggerModuleTest {

  private static final Logger LOGGER = Logger.getLogger(FileLoggerModuleTest.class);

  private static final String OUTPUT_FILE = "output.txt";
  private static final String ERROR_FILE = "error.txt";

  private static final String MESSAGE = "MESSAGE";
  private static final String ERROR = "ERROR";


  
  @Test
  public void testFileMessage() throws IOException {

    
    final FileLoggerModule fileLoggerModule = FileLoggerModule.getSingleton();

    
    fileLoggerModule.prepare();

    
    fileLoggerModule.printString(MESSAGE);

    
    assertEquals(readFirstLine(OUTPUT_FILE), MESSAGE);

    
    fileLoggerModule.unprepare();
  }

  
  @Test
  public void testNoFileMessage() throws IOException {

    
    final FileLoggerModule fileLoggerModule = FileLoggerModule.getSingleton();

    
    fileLoggerModule.prepare();

    
    assertEquals(readFirstLine(OUTPUT_FILE), null);

    
    fileLoggerModule.unprepare();
  }

  
  @Test
  public void testFileErrorMessage() throws FileNotFoundException {

    
    final FileLoggerModule fileLoggerModule = FileLoggerModule.getSingleton();

    
    fileLoggerModule.prepare();

    
    fileLoggerModule.printErrorString(ERROR);

    
    assertEquals(readFirstLine(ERROR_FILE), ERROR);

    
    fileLoggerModule.unprepare();
  }

  
  @Test
  public void testNoFileErrorMessage() throws FileNotFoundException {

    
    final FileLoggerModule fileLoggerModule = FileLoggerModule.getSingleton();

    
    fileLoggerModule.prepare();

    
    assertEquals(readFirstLine(ERROR_FILE), null);

    
    fileLoggerModule.unprepare();
  }

  
  private static final String readFirstLine(final String file) {

    String firstLine = null;
    BufferedReader bufferedReader = null;
    try {

      
      bufferedReader = new BufferedReader(new FileReader(file));

      while (bufferedReader.ready()) {

        
        firstLine = bufferedReader.readLine();
      }

      LOGGER.info("ModuleTest::readFirstLine() : firstLine : " + firstLine);

    } catch (final IOException e) {
      LOGGER.error("ModuleTest::readFirstLine()", e);
    } finally {

      if (bufferedReader != null) {
        try {
          bufferedReader.close();
        } catch (final IOException e) {
          LOGGER.error("ModuleTest::readFirstLine()", e);
        }
      }
    }

    return firstLine;
  }
}
