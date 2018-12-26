
package com.iluwatar.reader.writer.lock;

import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Writer implements Runnable {

  private static final Logger LOGGER = LoggerFactory.getLogger(Writer.class);

  private Lock writeLock;

  private String name;
  
  private long writingTime;

  
  public Writer(String name, Lock writeLock) {
    this(name, writeLock, 250L);
  }
  
  
  public Writer(String name, Lock writeLock, long writingTime) {
    this.name = name;
    this.writeLock = writeLock;
    this.writingTime = writingTime;
  }


  @Override
  public void run() {
    writeLock.lock();
    try {
      write();
    } catch (InterruptedException e) {
      LOGGER.info("InterruptedException when writing", e);
      Thread.currentThread().interrupt();
    } finally {
      writeLock.unlock();
    }
  }
  
  
  public void write() throws InterruptedException {
    LOGGER.info("{} begin", name);
    Thread.sleep(writingTime);
    LOGGER.info("{} finished after writing {}ms", name, writingTime);
  }
}
