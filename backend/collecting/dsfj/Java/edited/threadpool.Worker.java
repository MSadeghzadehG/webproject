
package com.iluwatar.threadpool;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Worker implements Runnable {

  private static final Logger LOGGER = LoggerFactory.getLogger(Worker.class);

  private final Task task;

  public Worker(final Task task) {
    this.task = task;
  }

  @Override
  public void run() {
    LOGGER.info("{} processing {}", Thread.currentThread().getName(), task.toString());
    try {
      Thread.sleep(task.getTimeMs());
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }
}
