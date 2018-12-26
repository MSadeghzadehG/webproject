
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


public class ReaderTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(Reader.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  private static final Logger LOGGER = LoggerFactory.getLogger(ReaderTest.class);

  
  @Test
  public void testRead() throws Exception {

    ExecutorService executeService = Executors.newFixedThreadPool(2);
    ReaderWriterLock lock = new ReaderWriterLock();

    Reader reader1 = spy(new Reader("Reader 1", lock.readLock()));
    Reader reader2 = spy(new Reader("Reader 2", lock.readLock()));

    executeService.submit(reader1);
    Thread.sleep(150);
    executeService.submit(reader2);

    executeService.shutdown();
    try {
      executeService.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      LOGGER.error("Error waiting for ExecutorService shutdown", e);
    }

            assertTrue(appender.logContains("Reader 1 begin"));
    assertTrue(appender.logContains("Reader 2 begin"));
    assertTrue(appender.logContains("Reader 1 finish"));
    assertTrue(appender.logContains("Reader 2 finish"));
  }
}
