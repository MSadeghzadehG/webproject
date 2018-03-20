
package com.iluwatar.delegation.simple;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import com.iluwatar.delegation.simple.printers.CanonPrinter;
import com.iluwatar.delegation.simple.printers.EpsonPrinter;
import com.iluwatar.delegation.simple.printers.HpPrinter;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class DelegateTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  private static final String MESSAGE = "Test Message Printed";

  @Test
  public void testCanonPrinter() throws Exception {
    PrinterController printerController = new PrinterController(new CanonPrinter());
    printerController.print(MESSAGE);

    assertEquals("Canon Printer : Test Message Printed", appender.getLastMessage());
  }

  @Test
  public void testHpPrinter() throws Exception {
    PrinterController printerController = new PrinterController(new HpPrinter());
    printerController.print(MESSAGE);

    assertEquals("HP Printer : Test Message Printed", appender.getLastMessage());
  }

  @Test
  public void testEpsonPrinter() throws Exception {
    PrinterController printerController = new PrinterController(new EpsonPrinter());
    printerController.print(MESSAGE);

    assertEquals("Epson Printer : Test Message Printed", appender.getLastMessage());
  }

  
  private class InMemoryAppender extends AppenderBase<ILoggingEvent> {

    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender() {
      ((Logger) LoggerFactory.getLogger("root")).addAppender(this);
      start();
    }

    @Override
    protected void append(ILoggingEvent eventObject) {
      log.add(eventObject);
    }

    public String getLastMessage() {
      return log.get(log.size() - 1).getFormattedMessage();
    }

    public int getLogSize() {
      return log.size();
    }
  }

}
