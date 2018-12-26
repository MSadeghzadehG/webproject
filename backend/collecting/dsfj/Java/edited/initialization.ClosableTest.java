
package com.iluwatar.resource.acquisition.is.initialization;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import java.util.LinkedList;
import java.util.List;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import static org.junit.jupiter.api.Assertions.assertTrue;


public class ClosableTest {

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
  public void testOpenClose() throws Exception {
    try (final SlidingDoor door = new SlidingDoor(); final TreasureChest chest = new TreasureChest()) {
      assertTrue(appender.logContains("Sliding door opens."));
      assertTrue(appender.logContains("Treasure chest opens."));
    }
    assertTrue(appender.logContains("Treasure chest closes."));
    assertTrue(appender.logContains("Sliding door closes."));
  }

  
  public class InMemoryAppender extends AppenderBase<ILoggingEvent> {
    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender() {
      ((Logger) LoggerFactory.getLogger("root")).addAppender(this);
      start();
    }

    @Override
    protected void append(ILoggingEvent eventObject) {
      log.add(eventObject);
    }

    public boolean logContains(String message) {
      return log.stream().anyMatch(event -> event.getMessage().equals(message));
    }
  }

}
