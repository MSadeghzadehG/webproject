
package com.iluwatar.event.aggregator;

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


public class KingJoffreyTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(KingJoffrey.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  @Test
  public void testOnEvent() {
    final KingJoffrey kingJoffrey = new KingJoffrey();

    for (int i = 0; i < Event.values().length; ++i) {
      assertEquals(i, appender.getLogSize());
      Event event = Event.values()[i];
      kingJoffrey.onEvent(event);

      final String expectedMessage = "Received event from the King's Hand: " + event.toString();
      assertEquals(expectedMessage, appender.getLastMessage());
      assertEquals(i + 1, appender.getLogSize());
    }

  }

  private class InMemoryAppender extends AppenderBase<ILoggingEvent> {
    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender(Class clazz) {
      ((Logger) LoggerFactory.getLogger(clazz)).addAppender(this);
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
