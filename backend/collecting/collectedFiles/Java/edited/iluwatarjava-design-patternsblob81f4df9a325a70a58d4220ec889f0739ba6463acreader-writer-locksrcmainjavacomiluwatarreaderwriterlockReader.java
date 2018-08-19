
package com.iluwatar.reader.writer.lock;

import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Reader implements Runnable {

  private static final Logger LOGGER = LoggerFactory.getLogger(Reader.class);

  private Lock readLock;

  private String name;
  
  private long readingTime;

  
  public Reader(String name, Lock readLock, long readingTime) {
    this.name = name;
    this.readLock = readLock;
    this.readingTime = readingTime;
  }
  
  
  public Reader(String name, Lock readLock) {
    this(name, readLock, 250L);
  }

  @Override
  public void run() {
    readLock.lock();
    try {
      read();
    } catch (InterruptedException e) {
      LOGGER.info("InterruptedException when reading", e);
      Thread.currentThread().interrupt();
    } finally {
      readLock.unlock();
    }
  }

  
  public void read() throws InterruptedException {
    LOGGER.info("{} begin", name);
    Thread.sleep(readingTime);
    LOGGER.info("{} finish after reading {}ms", name, readingTime);
  }
}
