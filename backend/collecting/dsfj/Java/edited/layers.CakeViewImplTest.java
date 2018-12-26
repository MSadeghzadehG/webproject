
package com.iluwatar.layers;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;


public class CakeViewImplTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(CakeViewImpl.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  @Test
  public void testRender() {

    final List<CakeLayerInfo> layers = new ArrayList<>();
    layers.add(new CakeLayerInfo("layer1", 1000));
    layers.add(new CakeLayerInfo("layer2", 2000));
    layers.add(new CakeLayerInfo("layer3", 3000));

    final List<CakeInfo> cakes = new ArrayList<>();
    final CakeInfo cake = new CakeInfo(new CakeToppingInfo("topping", 1000), layers);
    cakes.add(cake);

    final CakeBakingService bakingService = mock(CakeBakingService.class);
    when(bakingService.getAllCakes()).thenReturn(cakes);

    final CakeViewImpl cakeView = new CakeViewImpl(bakingService);

    assertEquals(0, appender.getLogSize());

    cakeView.render();
    assertEquals(cake.toString(), appender.getLastMessage());

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
