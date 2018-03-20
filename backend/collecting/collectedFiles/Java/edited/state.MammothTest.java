
package com.iluwatar.state;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class MammothTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  @Test
  public void testTimePasses() {
    final Mammoth mammoth = new Mammoth();

    mammoth.observe();
    assertEquals("The mammoth is calm and peaceful.", appender.getLastMessage());
    assertEquals(1 , appender.getLogSize());

    mammoth.timePasses();
    assertEquals("The mammoth gets angry!", appender.getLastMessage());
    assertEquals(2 , appender.getLogSize());

    mammoth.observe();
    assertEquals("The mammoth is furious!", appender.getLastMessage());
    assertEquals(3 , appender.getLogSize());

    mammoth.timePasses();
    assertEquals("The mammoth calms down.", appender.getLastMessage());
    assertEquals(4 , appender.getLogSize());

    mammoth.observe();
    assertEquals("The mammoth is calm and peaceful.", appender.getLastMessage());
    assertEquals(5 , appender.getLogSize());

  }

  
  @Test
  public void testToString() {
    final String toString = new Mammoth().toString();
    assertNotNull(toString);
    assertEquals("The mammoth", toString);
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

    public int getLogSize() {
      return log.size();
    }

    public String getLastMessage() {
      return log.get(log.size() - 1).getFormattedMessage();
    }
  }

}
