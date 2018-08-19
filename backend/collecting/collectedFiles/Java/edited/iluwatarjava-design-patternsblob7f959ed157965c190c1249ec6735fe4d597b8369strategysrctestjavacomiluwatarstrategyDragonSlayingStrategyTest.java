
package com.iluwatar.strategy;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class DragonSlayingStrategyTest {

  
  static Collection<Object[]> dataProvider() {
    return Arrays.asList(
        new Object[]{
            new MeleeStrategy(),
            "With your Excalibur you sever the dragon's head!"
        },
        new Object[]{
            new ProjectileStrategy(),
            "You shoot the dragon with the magical crossbow and it falls dead on the ground!"
        },
        new Object[]{
            new SpellStrategy(),
            "You cast the spell of disintegration and the dragon vaporizes in a pile of dust!"
        }
    );
  }

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }


  
  @ParameterizedTest
  @MethodSource("dataProvider")
  public void testExecute(DragonSlayingStrategy strategy, String expectedResult) {
    strategy.execute();
    assertEquals(expectedResult, appender.getLastMessage());
    assertEquals(1, appender.getLogSize());
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
