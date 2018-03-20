

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


public class ReaderAndWriterTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender();
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  private static final Logger LOGGER = LoggerFactory.getLogger(ReaderAndWriterTest.class);

  
  @Test
  public void testReadAndWrite() throws Exception {

    ReaderWriterLock lock = new ReaderWriterLock();

    Reader reader1 = new Reader("Reader 1", lock.readLock());
    Writer writer1 = new Writer("Writer 1", lock.writeLock());

    ExecutorService executeService = Executors.newFixedThreadPool(2);
    executeService.submit(reader1);
        Thread.sleep(150);
    executeService.submit(writer1);

    executeService.shutdown();
    try {
      executeService.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      LOGGER.error("Error waiting for ExecutorService shutdown", e);
    }

    assertTrue(appender.logContains("Reader 1 begin"));
    assertTrue(appender.logContains("Reader 1 finish"));
    assertTrue(appender.logContains("Writer 1 begin"));
    assertTrue(appender.logContains("Writer 1 finish"));
  }

  
  @Test
  public void testWriteAndRead() throws Exception {

    ExecutorService executeService = Executors.newFixedThreadPool(2);
    ReaderWriterLock lock = new ReaderWriterLock();

    Reader reader1 = new Reader("Reader 1", lock.readLock());
    Writer writer1 = new Writer("Writer 1", lock.writeLock());

    executeService.submit(writer1);
        Thread.sleep(150);
    executeService.submit(reader1);

    executeService.shutdown();
    try {
      executeService.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      LOGGER.error("Error waiting for ExecutorService shutdown", e);
    }

    assertTrue(appender.logContains("Writer 1 begin"));
    assertTrue(appender.logContains("Writer 1 finish"));
    assertTrue(appender.logContains("Reader 1 begin"));
    assertTrue(appender.logContains("Reader 1 finish"));
  }
}

