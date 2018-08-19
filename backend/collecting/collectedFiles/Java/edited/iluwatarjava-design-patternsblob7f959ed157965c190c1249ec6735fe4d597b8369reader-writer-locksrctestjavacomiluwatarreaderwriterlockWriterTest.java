
package com.iluwatar.reader.writer.lock;

import com.iluwatar.reader.writer.lock.utils.InMemoryAppender;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.Mockito.spy;


public class WriterTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(Writer.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  private static final Logger LOGGER = LoggerFactory.getLogger(WriterTest.class);

  
  @Test
  public void testWrite() throws Exception {

    ExecutorService executeService = Executors.newFixedThreadPool(2);
    ReaderWriterLock lock = new ReaderWriterLock();

    Writer writer1 = spy(new Writer("Writer 1", lock.writeLock()));
    Writer writer2 = spy(new Writer("Writer 2", lock.writeLock()));

    executeService.submit(writer1);
        Thread.sleep(150);
    executeService.submit(writer2);

    executeService.shutdown();
    try {
      executeService.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      LOGGER.error("Error waiting for ExecutorService shutdown", e);
    }
                assertTrue(appender.logContains("Writer 1 begin"));
    assertTrue(appender.logContains("Writer 1 finish"));
    assertTrue(appender.logContains("Writer 2 begin"));
    assertTrue(appender.logContains("Writer 2 finish"));
  }
}
