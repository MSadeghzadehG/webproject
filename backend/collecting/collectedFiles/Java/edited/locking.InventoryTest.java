
package com.iluwatar.doublechecked.locking;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTimeout;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class InventoryTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(Inventory.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  
  private static final int THREAD_COUNT = 8;

  
  private static final int INVENTORY_SIZE = 1000;

  
  @Test
  public void testAddItem() throws Exception {
    assertTimeout(ofMillis(10000), () -> {
            final Inventory inventory = new Inventory(INVENTORY_SIZE);
      final ExecutorService executorService = Executors.newFixedThreadPool(THREAD_COUNT);
      for (int i = 0; i < THREAD_COUNT; i++) {
        executorService.execute(() -> {
          while (inventory.addItem(new Item())) {};
        });
      }

            executorService.shutdown();
      executorService.awaitTermination(5, TimeUnit.SECONDS);

            final List<Item> items = inventory.getItems();
      assertNotNull(items);
      assertEquals(INVENTORY_SIZE, items.size());

      assertEquals(INVENTORY_SIZE, appender.getLogSize());

            for (int i = 0; i < items.size(); i++) {
        assertTrue(appender.log.get(i).getFormattedMessage().contains("items.size()=" + (i + 1)));
      }
    });
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

    public int getLogSize() {
      return log.size();
    }
  }

}
